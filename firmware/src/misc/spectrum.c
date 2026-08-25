/*
 * spectrum.c — 24-bar real-time spectrum analyser display
 *
 * Rendering strategy:
 *   • Axes and labels are drawn ONCE by Spectrum_DrawAxes().
 *   • Spectrum_Update() uses differential rendering:
 *     – compares each bar's new pixel-height with its cached previous height
 *     – only repaints the strip of pixels that actually changed
 *     – a bar that didn't move costs zero SPI traffic
 *
 * This typically touches ~4 800 pixels per frame (vs 76 800 for a full
 * redraw), giving sub-2 ms update times at 8 MHz SPI.
 */

#include "spectrum.h"
#include "drivers/ili9341.h"

/* ==== embedded 5×7 bitmap font (digits 0-9 only) ======================= */
/* Each glyph is 5 columns × 7 rows, stored column-major: font5x7[ch][col].
   Bit 0 = top row, bit 6 = bottom row.  '1' = foreground pixel.           */

const uint8_t font5x7[10][5] = {
    /* 0 */ {0x3E, 0x51, 0x49, 0x45, 0x3E},
    /* 1 */ {0x00, 0x42, 0x7F, 0x40, 0x00},
    /* 2 */ {0x42, 0x61, 0x51, 0x49, 0x46},
    /* 3 */ {0x21, 0x41, 0x45, 0x4B, 0x31},
    /* 4 */ {0x18, 0x14, 0x12, 0x7F, 0x10},
    /* 5 */ {0x27, 0x45, 0x45, 0x45, 0x39},
    /* 6 */ {0x3C, 0x4A, 0x49, 0x49, 0x30},
    /* 7 */ {0x01, 0x71, 0x09, 0x05, 0x03},
    /* 8 */ {0x36, 0x49, 0x49, 0x49, 0x36},
    /* 9 */ {0x06, 0x49, 0x49, 0x29, 0x1E},
};

/* ---- draw a single digit at (x, y) in the given colour ---------------- */
static void draw_digit(uint16_t x, uint16_t y, uint8_t digit, uint16_t color)
{
    if (digit > 9) return;
    const uint8_t *glyph = font5x7[digit];

    for (uint8_t col = 0; col < 5; ++col) {
        uint8_t bits = glyph[col];
        for (uint8_t row = 0; row < 7; ++row) {
            if (bits & (1u << row))
                ILI9341_DrawPixel(x + col, y + row, color);
        }
    }
}

/* ---- draw an unsigned integer (up to 3 digits) right-aligned ---------- */
/* x_right = pixel column of the rightmost digit's right edge              */
static void draw_number_right(uint16_t x_right, uint16_t y,
                               uint16_t value, uint16_t color)
{
    /* decompose into up to 3 digits */
    uint8_t digits[3];
    uint8_t n = 0;

    if (value == 0) {
        digits[n++] = 0;
    } else {
        uint16_t v = value;
        while (v > 0 && n < 3) {
            digits[n++] = v % 10;
            v /= 10;
        }
    }

    /* draw right-to-left, each digit is 5px wide + 1px gap */
    uint16_t cx = x_right;
    for (uint8_t i = 0; i < n; ++i) {
        cx -= 5;
        draw_digit(cx, y, digits[i], color);
        cx -= 1;  /* 1px inter-digit gap */
    }
}

/* ---- draw an unsigned integer left-aligned ----------------------------- */
static void draw_number_left(uint16_t x, uint16_t y,
                              uint16_t value, uint16_t color)
{
    uint8_t digits[3];
    uint8_t n = 0;

    if (value == 0) {
        digits[n++] = 0;
    } else {
        uint16_t v = value;
        while (v > 0 && n < 3) {
            digits[n++] = v % 10;
            v /= 10;
        }
    }

    /* digits[] is in reverse order; draw from MSB to LSB */
    uint16_t cx = x;
    for (int i = (int)n - 1; i >= 0; --i) {
        draw_digit(cx, y, digits[i], color);
        cx += 6;  /* 5px glyph + 1px gap */
    }
}

/* ==== draw static axes and labels ====================================== */

void Spectrum_DrawAxes(void)
{
    /* Black background — full screen clear */
    ILI9341_FillScreen(COLOR_BLACK);

    /* ---- Y-axis (vertical line at the left edge of the plot) ---------- */
    ILI9341_DrawVLine(PLOT_X_ORIGIN, PLOT_Y_TOP, PLOT_HEIGHT + 1, COLOR_PURPLE);

    /* Y-axis tick marks and labels: 0, 64, 128, 192, 255 */
    static const uint16_t y_vals[] = {0, 64, 128, 192, 255};
    for (uint8_t i = 0; i < 5; ++i) {
        /* Map magnitude value to pixel row (0 = bottom, 255 = top) */
        uint16_t py = AXIS_Y - (uint16_t)((uint32_t)y_vals[i] * PLOT_HEIGHT / 255);

        /* 4px horizontal tick mark */
        ILI9341_DrawHLine(PLOT_X_ORIGIN - 4, py, 4, COLOR_PURPLE);

        /* Numeric label, right-aligned to the left of the tick */
         draw_number_right(PLOT_X_ORIGIN - 5, py - 3, y_vals[i], COLOR_PURPLE);
    }

    /* ---- X-axis (horizontal line at the bottom of the plot) ----------- */
    ILI9341_DrawHLine(PLOT_X_ORIGIN, AXIS_Y, 320 - PLOT_X_ORIGIN, COLOR_PURPLE);

    /* X-axis tick marks and labels: every bar gets a tick, every 4th a number */
    for (uint8_t b = 0; b < SPECTRUM_BARS; ++b) {
        uint16_t bx = BAR_X_START + b * BAR_SLOT_WIDTH + BAR_WIDTH / 2;

        /* 3px vertical tick below the axis */
        ILI9341_DrawVLine(bx, AXIS_Y + 1, 3, COLOR_PURPLE);

        /* label every 4th bar: 1, 5, 9, 13, 17, 21 */
        if ((b % 4) == 0) {
            draw_number_left(bx - 2, AXIS_Y + 6, b + 1, COLOR_PURPLE);
        }
    }
}

/* ==== differential bar rendering ======================================= */

/* Cached pixel heights from the previous frame (0 = bar empty) */
static uint16_t prev_heights[SPECTRUM_BARS];

void Spectrum_Update(const uint8_t *magnitudes)
{
    for (uint8_t i = 0; i < SPECTRUM_BARS; ++i) {
        /* Scale 0-255 magnitude → 0-PLOT_HEIGHT pixels */
        uint16_t new_h = (uint16_t)((uint32_t)magnitudes[i] * PLOT_HEIGHT / 255);
        uint16_t old_h = prev_heights[i];

        if (new_h == old_h) continue;      /* no change → skip entirely */

        uint16_t bx = BAR_X_START + (uint16_t)i * BAR_SLOT_WIDTH;

        if (new_h > old_h) {
            /* Bar grew: paint GREEN in the newly-exposed region */
            uint16_t y_top = AXIS_Y - new_h;
            uint16_t fill_h = new_h - old_h;
            ILI9341_FillRect(bx, y_top, BAR_WIDTH, fill_h, COLOR_GREEN);
        } else {
            /* Bar shrank: erase (BLACK) the region that is no longer bar */
            uint16_t y_top = AXIS_Y - old_h;
            uint16_t fill_h = old_h - new_h;
            ILI9341_FillRect(bx, y_top, BAR_WIDTH, fill_h, COLOR_BLACK);
        }

        prev_heights[i] = new_h;
    }
}
