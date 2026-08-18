/*
 * ili9341.c — ILI9341 TFT display driver for STM32F303VCT6
 *
 * Two rendering pipelines:
 *   Pipeline 1 — Blocking:  CPU polls SPI TXE and pushes pixels one by one.
 *   Pipeline 2 — DMA:       DMA1_Channel5 streams a buffer to SPI2_TX;
 *                            the CPU is free during the transfer.
 *
 * Hardware wiring (directly from implementation plan):
 *   SPI2:  PB13 = SCK, PB14 = MISO, PB15 = MOSI  (already configured)
 *   GPIO:  PD11 = CS (active-low)
 *          PD12 = DC (0 = command, 1 = data)
 *          PD13 = RST (active-low reset)
 *
 * References: ILI9341 datasheet v1.11, RM0316 §28 (SPI) / §13 (DMA)
 */

#include "ili9341.h"
#include "spi.h"
#include "delay.h"

/* ==== pin definitions ================================================== */
#define ILI_CS_PIN    11u    /* Port D */
#define ILI_DC_PIN    12u
#define ILI_RST_PIN   13u

/* ---- inline pin helpers ----------------------------------------------- */
static inline void ILI_Select(void)   { GPIOD->BRR  = (1u << ILI_CS_PIN);  }
static inline void ILI_Deselect(void) { GPIOD->BSRR = (1u << ILI_CS_PIN);  }
static inline void ILI_CmdMode(void)  { GPIOD->BRR  = (1u << ILI_DC_PIN);  }
static inline void ILI_DataMode(void) { GPIOD->BSRR = (1u << ILI_DC_PIN);  }
static inline void ILI_RstLow(void)   { GPIOD->BRR  = (1u << ILI_RST_PIN); }
static inline void ILI_RstHigh(void)  { GPIOD->BSRR = (1u << ILI_RST_PIN); }

/* ==== current screen dimensions (updated by SetRotation) =============== */
static uint16_t ili_width  = ILI9341_WIDTH;
static uint16_t ili_height = ILI9341_HEIGHT;

/* ==== ILI9341 command bytes ============================================ */
#define ILI_CMD_NOP         0x00
#define ILI_CMD_SWRESET     0x01
#define ILI_CMD_SLPOUT      0x11
#define ILI_CMD_DISPOFF     0x28
#define ILI_CMD_DISPON      0x29
#define ILI_CMD_CASET       0x2A   /* Column Address Set   */
#define ILI_CMD_PASET       0x2B   /* Page Address Set     */
#define ILI_CMD_RAMWR       0x2C   /* Memory Write         */
#define ILI_CMD_MADCTL      0x36   /* Memory Access Ctrl   */
#define ILI_CMD_PIXFMT      0x3A   /* Pixel Format Set     */
#define ILI_CMD_FRMCTR1     0xB1   /* Frame Rate Control   */
#define ILI_CMD_DFUNCTR     0xB6   /* Display Function Ctrl*/
#define ILI_CMD_PWCTR1      0xC0   /* Power Control 1      */
#define ILI_CMD_PWCTR2      0xC1   /* Power Control 2      */
#define ILI_CMD_VMCTR1      0xC5   /* VCOM Control 1       */
#define ILI_CMD_VMCTR2      0xC7   /* VCOM Control 2       */
#define ILI_CMD_PWCTRA      0xCB   /* Power Control A      */
#define ILI_CMD_PWCTRB      0xCF   /* Power Control B      */
#define ILI_CMD_DTMCTRA     0xE8   /* Driver Timing Ctrl A */
#define ILI_CMD_DTMCTRB     0xEA   /* Driver Timing Ctrl B */
#define ILI_CMD_PONSEQCTR   0xED   /* Power-On Seq Control */
#define ILI_CMD_PUMPRATIO   0xF7   /* Pump Ratio Control   */
#define ILI_CMD_PGAMCTR     0xE0   /* Positive Gamma Corr. */
#define ILI_CMD_NGAMCTR     0xE1   /* Negative Gamma Corr. */

/* ==== low-level command / data transport =============================== */

/* Send a single command byte (DC=0, 8-bit SPI) */
static void ILI_WriteCmd(uint8_t cmd)
{
    ILI_CmdMode();
    SPI_Tx8(cmd);
}

/* Send a single data byte (DC=1, 8-bit SPI) */
static void ILI_WriteData8(uint8_t data)
{
    ILI_DataMode();
    SPI_Tx8(data);
}

/* Send multiple data bytes (DC=1, 8-bit SPI) — used for init params */
static void ILI_WriteData8_Multi(const uint8_t *p, uint16_t n)
{
    ILI_DataMode();
    while (n--) {
        SPI_Tx8(*p++);
    }
}

/* ==== GPIO setup for ILI9341 control pins ============================== */
static void ILI_GPIO_Init(void)
{
    /* Port D clock should already be enabled by SPI_Init (PD8-10 for MAX7219),
       but ensure it here too in case ILI9341 is used standalone. */
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;
    (void)RCC->AHBENR;

    /* PD11, PD12, PD13: general-purpose output, push-pull, high speed */
    for (unsigned p = ILI_CS_PIN; p <= ILI_RST_PIN; ++p) {
        GPIOD->MODER   = (GPIOD->MODER   & ~(3u << (2*p))) | (1u << (2*p));
        GPIOD->OSPEEDR = (GPIOD->OSPEEDR & ~(3u << (2*p))) | (3u << (2*p));
        GPIOD->OTYPER  &= ~(1u << p);           /* push-pull          */
        GPIOD->PUPDR   &= ~(3u << (2*p));       /* no pull            */
    }

    /* Default idle state: CS high (deselected), RST high */
    ILI_Deselect();
    ILI_RstHigh();
}

/* ==== hardware reset sequence ========================================== */
void ILI9341_Reset(void)
{
    ILI_RstHigh();
    delay_ms(5);
    ILI_RstLow();
    delay_ms(20);       /* RST must be held low ≥ 10 µs (datasheet §11.1) */
    ILI_RstHigh();
    delay_ms(150);      /* wait for ILI9341 to come out of reset           */
}

/* ==== full initialization sequence ===================================== */
/* Every step is annotated with the ILI9341 datasheet command reference.   */

void ILI9341_Init(void)
{
    ILI_GPIO_Init();

    /* Switch SPI2 to 8-bit mode for the init command sequence */
    SPI_SetDataSize8();

    /* --- hardware reset ------------------------------------------------ */
    ILI9341_Reset();

    ILI_Select();

    /* --- undocumented / extended power-on sequence ---------------------- */
    /* These commands come from virtually every ILI9341 init code and the
       "Application Note" addendum. They tune the internal charge pump
       and driver timing for the specific panel.                           */

    /* Power Control B (0xCF) */
    ILI_WriteCmd(ILI_CMD_PWCTRB);
    { static const uint8_t d[] = {0x00, 0xC1, 0x30}; ILI_WriteData8_Multi(d, 3); }

    /* Power-On Sequence Control (0xED) */
    ILI_WriteCmd(ILI_CMD_PONSEQCTR);
    { static const uint8_t d[] = {0x64, 0x03, 0x12, 0x81}; ILI_WriteData8_Multi(d, 4); }

    /* Driver Timing Control A (0xE8) */
    ILI_WriteCmd(ILI_CMD_DTMCTRA);
    { static const uint8_t d[] = {0x85, 0x00, 0x78}; ILI_WriteData8_Multi(d, 3); }

    /* Power Control A (0xCB) */
    ILI_WriteCmd(ILI_CMD_PWCTRA);
    { static const uint8_t d[] = {0x39, 0x2C, 0x00, 0x34, 0x02}; ILI_WriteData8_Multi(d, 5); }

    /* Pump Ratio Control (0xF7) */
    ILI_WriteCmd(ILI_CMD_PUMPRATIO);
    ILI_WriteData8(0x20);

    /* Driver Timing Control B (0xEA) */
    ILI_WriteCmd(ILI_CMD_DTMCTRB);
    { static const uint8_t d[] = {0x00, 0x00}; ILI_WriteData8_Multi(d, 2); }

    /* --- documented power & VCOM --------------------------------------- */

    /* Power Control 1 (0xC0): GVDD = 4.60 V */
    ILI_WriteCmd(ILI_CMD_PWCTR1);
    ILI_WriteData8(0x23);

    /* Power Control 2 (0xC1): step-up factor */
    ILI_WriteCmd(ILI_CMD_PWCTR2);
    ILI_WriteData8(0x10);

    /* VCOM Control 1 (0xC5): VMH = 3.45 V, VML = -1.5 V */
    ILI_WriteCmd(ILI_CMD_VMCTR1);
    { static const uint8_t d[] = {0x3E, 0x28}; ILI_WriteData8_Multi(d, 2); }

    /* VCOM Control 2 (0xC7): VCOMH offset */
    ILI_WriteCmd(ILI_CMD_VMCTR2);
    ILI_WriteData8(0x86);

    /* --- memory access & pixel format ---------------------------------- */

    /* Memory Access Control (0x36): default portrait, RGB order */
    ILI_WriteCmd(ILI_CMD_MADCTL);
    ILI_WriteData8(0x48);       /* MX=0, MY=1, MV=0, BGR=1 for most modules */

    /* Pixel Format Set (0x3A): 16 bits/pixel (RGB565) */
    ILI_WriteCmd(ILI_CMD_PIXFMT);
    ILI_WriteData8(0x55);       /* DPI=16bit, DBI=16bit */

    /* Frame Rate Control (0xB1): 70 Hz (default for ILI9341) */
    ILI_WriteCmd(ILI_CMD_FRMCTR1);
    { static const uint8_t d[] = {0x00, 0x1B}; ILI_WriteData8_Multi(d, 2); }

    /* Display Function Control (0xB6) */
    ILI_WriteCmd(ILI_CMD_DFUNCTR);
    { static const uint8_t d[] = {0x08, 0x82, 0x27}; ILI_WriteData8_Multi(d, 3); }

    /* --- gamma correction ---------------------------------------------- */

    /* Positive Gamma Correction (0xE0) */
    ILI_WriteCmd(ILI_CMD_PGAMCTR);
    {
        static const uint8_t d[] = {
            0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
            0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00
        };
        ILI_WriteData8_Multi(d, 15);
    }

    /* Negative Gamma Correction (0xE1) */
    ILI_WriteCmd(ILI_CMD_NGAMCTR);
    {
        static const uint8_t d[] = {
            0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
            0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F
        };
        ILI_WriteData8_Multi(d, 15);
    }

    /* --- wake up and turn on ------------------------------------------- */

    /* Sleep Out (0x11) — must wait ≥ 120 ms before sending next command */
    ILI_WriteCmd(ILI_CMD_SLPOUT);
    SPI_WaitDone();
    delay_ms(120);

    /* Display ON (0x29) */
    ILI_WriteCmd(ILI_CMD_DISPON);
    SPI_WaitDone();

    ILI_Deselect();

    /* Set dimensions to portrait default */
    ili_width  = ILI9341_WIDTH;    /* 240 */
    ili_height = ILI9341_HEIGHT;   /* 320 */
}

/* ==== rotation ========================================================= */
/* MADCTL bits: MY=7, MX=6, MV=5, ML=4, BGR=3, MH=2                       */

void ILI9341_SetRotation(uint8_t r)
{
    uint8_t madctl;

    SPI_SetDataSize8();
    ILI_Select();
    ILI_WriteCmd(ILI_CMD_MADCTL);

    switch (r & 0x03) {
    case 0: /* landscape */
        madctl = 0x48;
        ili_width  = ILI9341_WIDTH;
        ili_height = ILI9341_HEIGHT;
        break;
    case 1: /* portrait */
        madctl = 0x28;
        ili_width  = ILI9341_HEIGHT;
        ili_height = ILI9341_WIDTH;
        break;
    case 2: /* portrait inverted */
        madctl = 0x88;
        ili_width  = ILI9341_WIDTH;
        ili_height = ILI9341_HEIGHT;
        break;
    case 3: /* landscape inverted */
        madctl = 0xE8;
        ili_width  = ILI9341_HEIGHT;
        ili_height = ILI9341_WIDTH;
        break;
    default:
        madctl = 0x48;
        break;
    }

    ILI_WriteData8(madctl);
    SPI_WaitDone();
    ILI_Deselect();
}

/* ==== coordinate windowing ============================================= */
/* Sets the column (x) and page (y) address range, then issues RAMWR.
   After this, every subsequent data byte is a pixel in the window.        */

void ILI9341_SetWindow(uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1)
{
    /* Column Address Set (0x2A): 4 bytes — SC[15:8], SC[7:0], EC[15:8], EC[7:0] */
    ILI_WriteCmd(ILI_CMD_CASET);
    ILI_DataMode();
    SPI_Tx8((uint8_t)(x0 >> 8));
    SPI_Tx8((uint8_t)(x0 & 0xFF));
    SPI_Tx8((uint8_t)(x1 >> 8));
    SPI_Tx8((uint8_t)(x1 & 0xFF));

    /* Page Address Set (0x2B): 4 bytes — SP[15:8], SP[7:0], EP[15:8], EP[7:0] */
    ILI_WriteCmd(ILI_CMD_PASET);
    ILI_DataMode();
    SPI_Tx8((uint8_t)(y0 >> 8));
    SPI_Tx8((uint8_t)(y0 & 0xFF));
    SPI_Tx8((uint8_t)(y1 >> 8));
    SPI_Tx8((uint8_t)(y1 & 0xFF));

    /* Memory Write (0x2C) — ready to receive pixel data */
    ILI_WriteCmd(ILI_CMD_RAMWR);
    ILI_DataMode();
}

/* ===================================================================== */
/*                PIPELINE 1 — BLOCKING (POLLING)                        */
/* ===================================================================== */

void ILI9341_FillScreen(uint16_t color)
{
    ILI9341_FillRect(0, 0, ili_width, ili_height, color);
}

void ILI9341_FillRect(uint16_t x, uint16_t y,
                      uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= ili_width || y >= ili_height) return;
    if (x + w > ili_width)  w = ili_width  - x;
    if (y + h > ili_height) h = ili_height - y;

    SPI_SetDataSize8();
    ILI_Select();
    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);

    SPI_SetDataSize16();
    uint32_t total = (uint32_t)w * h;
    while (total--) {
        SPI_Tx16(color);
    }

    SPI_WaitDone();
    ILI_Deselect();
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ili_width || y >= ili_height) return;

    SPI_SetDataSize8();
    ILI_Select();
    ILI9341_SetWindow(x, y, x, y);

    SPI_SetDataSize16();
    SPI_Tx16(color);

    SPI_WaitDone();
    ILI_Deselect();
}

void ILI9341_DrawVLine(uint16_t x, uint16_t y,
                       uint16_t h, uint16_t color)
{
    ILI9341_FillRect(x, y, 1, h, color);
}

void ILI9341_DrawHLine(uint16_t x, uint16_t y,
                       uint16_t w, uint16_t color)
{
    ILI9341_FillRect(x, y, w, 1, color);
}

/* ===================================================================== */
/*                PIPELINE 2 — DMA-DRIVEN                                */
/* ===================================================================== */
/* Strategy: for FillRect_DMA, we fill a small static row buffer with the  */
/* target color and DMA it out repeatedly in chunks. For PushBuffer_DMA    */
/* the caller supplies the pixel buffer directly.                          */
/*                                                                         */
/* DMA1_Channel5 max transfer count is 65535. For larger fills we chain    */
/* multiple DMA bursts from the TC ISR or from the foreground loop.        */

/* Row buffer: 240 pixels × 2 bytes = 480 bytes. Fits one full-width row. */
#define DMA_ROW_BUF_PX  240
static uint16_t dma_row_buf[DMA_ROW_BUF_PX];

extern volatile int g_spi_dma_busy;        /* defined in spi.c             */

void ILI9341_FillRect_DMA(uint16_t x, uint16_t y,
                          uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= ili_width || y >= ili_height) return;
    if (x + w > ili_width)  w = ili_width  - x;
    if (y + h > ili_height) h = ili_height - y;

    /* Fill the row buffer with the target color */
    uint16_t fill_w = (w <= DMA_ROW_BUF_PX) ? w : DMA_ROW_BUF_PX;
    for (uint16_t i = 0; i < fill_w; ++i)
        dma_row_buf[i] = color;

    SPI_SetDataSize8();
    ILI_Select();
    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);
    SPI_SetDataSize16();

    /* Stream out row-by-row using DMA.
       SPI_DMA_Start blocks if a previous transfer is still in flight. */
    uint32_t total = (uint32_t)w * h;
    while (total > 0) {
        uint16_t chunk = (total > fill_w) ? fill_w : (uint16_t)total;
        SPI_DMA_Start(dma_row_buf, chunk);
        total -= chunk;
        /* Wait for this chunk before starting the next (CS stays asserted) */
        while (SPI_DMA_Busy()) { }
    }

    SPI_WaitDone();
    ILI_Deselect();
}

void ILI9341_PushBuffer_DMA(uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h,
                            const uint16_t *buf)
{
    if (x >= ili_width || y >= ili_height) return;

    SPI_SetDataSize8();
    ILI_Select();
    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);
    SPI_SetDataSize16();

    uint32_t total = (uint32_t)w * h;
    const uint16_t *p = buf;

    /* DMA CNDTR is 16-bit → max 65535 transfers per burst */
    while (total > 0) {
        uint16_t chunk = (total > 65535u) ? 65535u : (uint16_t)total;
        SPI_DMA_Start(p, chunk);
        p     += chunk;
        total -= chunk;
        while (SPI_DMA_Busy()) { }
    }

    SPI_WaitDone();
    ILI_Deselect();
}

int ILI9341_DMA_Busy(void)
{
    return SPI_DMA_Busy();
}

/* ==== DMA1_Channel5 ISR ================================================ */
/* Called when the DMA transfer-complete flag fires. We clear the flag      */
/* and signal the foreground code that the DMA burst is done.               */

void DMA1_Channel5_IRQHandler(void)
{
    /* Clear all DMA1_Channel5 interrupt flags */
    DMA1->IFCR = DMA_IFCR_CTCIF5
               | DMA_IFCR_CHTIF5
               | DMA_IFCR_CTEIF5
               | DMA_IFCR_CGIF5;

    /* Disable the channel (single-shot; must be re-armed for next burst) */
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;

    /* Stop SPI from generating further DMA requests until next transfer */
    SPI2->CR2 &= ~SPI_CR2_TXDMAEN;

    /* Signal foreground */
    g_spi_dma_busy = 0;
}
