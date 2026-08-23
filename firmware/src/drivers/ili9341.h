#ifndef ILI9341_H
#define ILI9341_H

#include <stdint.h>

/* ---- screen dimensions (native portrait) ------------------------------ */
#define ILI9341_WIDTH   320
#define ILI9341_HEIGHT  240

/* ---- common RGB565 colors --------------------------------------------- */
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_BLUE      0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_RED       0x001F
#define COLOR_YELLOW    0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_CYAN      0xFFE0
#define COLOR_ORANGE    0x01FF
#define COLOR_GRAY      0x8410
#define COLOR_PURPLE    0x780F

/* Pack 8-bit R, G, B into a 16-bit RGB565 value */
#define RGB565(r, g, b) ((uint16_t)(((r) & 0xF8) << 8 | \
                                    ((g) & 0xFC) << 3 | \
                                    ((b) & 0xF8) >> 3))

/* ---- initialization & control ----------------------------------------- */
void ILI9341_Init(void);
void ILI9341_Reset(void);
void ILI9341_SetRotation(uint8_t r);       /* 0-3 → 0° / 90° / 180° / 270° */

/* ---- Pipeline 1: blocking (polling) drawing --------------------------- */
void ILI9341_SetWindow(uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_FillRect(uint16_t x, uint16_t y,
                      uint16_t w, uint16_t h, uint16_t color);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_DrawVLine(uint16_t x, uint16_t y,
                       uint16_t h, uint16_t color);
void ILI9341_DrawHLine(uint16_t x, uint16_t y,
                       uint16_t w, uint16_t color);

/* ---- Pipeline 2: DMA-driven bulk write -------------------------------- */
void ILI9341_FillRect_DMA(uint16_t x, uint16_t y,
                          uint16_t w, uint16_t h, uint16_t color);
void ILI9341_PushBuffer_DMA(uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h,
                            const uint16_t *buf);
int  ILI9341_DMA_Busy(void);

#endif
