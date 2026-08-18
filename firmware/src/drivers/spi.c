#include "spi.h"

/* ---- init: SPE off while configuring ------------------------------ */
void SPI_Init(void){
    /* Example shape: three SPI pins on one port, plus GPIO chip-select
   and control pins on another. Check YOUR device's alternate-function
   table for the correct AF number -- it varies by pin and instance. */

    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIODEN;

    /* --- SCK, MISO, MOSI: alternate function mode ---------------------- */
    for (int p = 13; p <= 15; ++p) {
        GPIOB->MODER   = (GPIOB->MODER   & ~(3u << (2*p))) | (2u << (2*p));
                                                    /* 10 = alternate fn */
        GPIOB->OTYPER &= ~(1u << p);                  /* push-pull         */
        GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~(3u << (2*p))) | (3u << (2*p));
                                                    /* HIGH SPEED        */
        GPIOB->PUPDR  &= ~(3u << (2*p));              /* no pulls          */
    }
    /* AFR[0] covers pins 0-7, AFR[1] covers pins 8-15, 4 bits each */
    GPIOB->AFR[1] |= (5u << ((13-8)*4)) | (5u << ((14-8)*4)) | (5u << ((15-8)*4));

    /* --- CS, DC, RST: ordinary outputs, also fast ---------------------- */
    for (int p = 8; p <= 10; ++p) {
        GPIOD->MODER   = (GPIOD->MODER   & ~(3u << (2*p))) | (1u << (2*p));
        GPIOD->OSPEEDR = (GPIOD->OSPEEDR & ~(3u << (2*p))) | (3u << (2*p));
    }
    GPIOD->BSRR = (1u << 8);      /* CS idle high = deselected  */
        
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    (void)RCC->APB1ENR;                         /* wait for clock to be ready */

    SPI2->CR1 = 0u;                       /* SPE = 0, clean slate      */

    SPI2->CR2 = (15u << 8)                 /* DS   = 1111 -> 16-bit      */
              | SPI_CR2_FRXTH;            /* RXNE at 8 bits            */

    SPI2->CR1 = SPI_CR1_MSTR              /* we are the master         */
              | SPI_CR1_SSM               /* software NSS...           */
              | SPI_CR1_SSI               /* ...held HIGH (Trap 3)     */
              | (2u << 3);                /* BR = 010 -> PCLK / 8     */
                                          /* CPOL = 0, CPHA = 0        */
    SPI2->CR1 |= SPI_CR1_SPE;
}

/* ---- the two accessors that must exist exactly once --------------- */
static inline void     SPI_Put(uint16_t b) { *(volatile uint16_t *)&SPI2->DR = b; }
static inline uint16_t  SPI_Get(void)      { return *(volatile uint16_t *)&SPI2->DR; }

/* ---- one byte out, one byte in (they happen together) ------------- */
uint16_t SPI_Xfer(uint16_t out)
{
    while (!(SPI2->SR & SPI_SR_TXE))  { }   /* room in TX FIFO?        */
    SPI_Put(out);
    while (!(SPI2->SR & SPI_SR_RXNE)) { }   /* wait for the echo back  */
    return SPI_Get();
}

/* ---- transmit-only: still must drain RX, or OVR sets -------------- */
void SPI_Write(const uint16_t *p, size_t n)
{
    while (n--) { (void)SPI_Xfer(*p++); } 
}

void SPI_WaitDone(void)
{
    while (SPI2->SR & SPI_SR_FTLVL) { }   /* 1. TX FIFO drained      */
    while (SPI2->SR & SPI_SR_BSY)   { }   /* 2. shift register empty */
    /* 3. Drain RX FIFO — use byte reads so we don't pop two entries
       when the peripheral is in 8-bit mode.                          */
    while (SPI2->SR & SPI_SR_FRLVL)
        { (void)*(volatile uint8_t *)&SPI2->DR; }
}

/* ==== data-size switching ============================================== */
/* The STM32F303 SPI requires SPE = 0 to change the DS field in CR2.      */

void SPI_SetDataSize8(void)
{
    SPI_WaitDone();
    SPI2->CR1 &= ~SPI_CR1_SPE;

    SPI2->CR2 = (SPI2->CR2 & ~(0xFu << 8))    /* clear DS[3:0]         */
              | (7u << 8)                       /* DS = 0111 → 8-bit    */
              | SPI_CR2_FRXTH;                  /* RXNE fires at 8 bits */

    SPI2->CR1 |= SPI_CR1_SPE;
}

void SPI_SetDataSize16(void)
{
    SPI_WaitDone();
    SPI2->CR1 &= ~SPI_CR1_SPE;

    SPI2->CR2 = (SPI2->CR2 & ~((0xFu << 8) | SPI_CR2_FRXTH))
              | (15u << 8);                     /* DS = 1111 → 16-bit   */
              /* FRXTH cleared: RXNE fires at 16 bits                   */

    SPI2->CR1 |= SPI_CR1_SPE;
}

/* ==== transmit-only helpers ============================================ */
/* These wait for TXE, push one frame, then drain any RX data to prevent   */
/* the OVR (overrun) flag from locking up the peripheral.                  */

void SPI_Tx8(uint8_t b)
{
    while (!(SPI2->SR & SPI_SR_TXE)) { }
    *(volatile uint8_t *)&SPI2->DR = b;         /* 8-bit write           */
    /* drain RX after shift completes */
    while (!(SPI2->SR & SPI_SR_RXNE)) { }
    (void)*(volatile uint8_t *)&SPI2->DR;
}

void SPI_Tx16(uint16_t w)
{
    while (!(SPI2->SR & SPI_SR_TXE)) { }
    *(volatile uint16_t *)&SPI2->DR = w;        /* 16-bit write          */
    while (!(SPI2->SR & SPI_SR_RXNE)) { }
    (void)*(volatile uint16_t *)&SPI2->DR;
}

/* ==== DMA-driven bulk transfer ========================================= */
/* Uses DMA1_Channel5 which is hard-wired to SPI2_TX on STM32F303.         */
/* The transfer-complete ISR (DMA1_Channel5_IRQHandler) lives in           */
/* ili9341.c so it can deassert CS after the last byte shifts out.          */

volatile int g_spi_dma_busy;     /* 1 while DMA is in flight              */

void SPI_DMA_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    (void)RCC->AHBENR;

    /* Disable channel while configuring */
    DMA1_Channel5->CCR = 0;

    /* Peripheral address: SPI2 data register */
    DMA1_Channel5->CPAR = (uint32_t)&SPI2->DR;

    /* Direction: memory → peripheral, memory increment, 16-bit both sides,
       transfer-complete interrupt enabled                                  */
    DMA1_Channel5->CCR = DMA_CCR_DIR            /* read from memory       */
                       | DMA_CCR_MINC           /* memory increment       */
                       | DMA_CCR_PSIZE_0        /* peripheral = 16-bit    */
                       | DMA_CCR_MSIZE_0        /* memory    = 16-bit     */
                       | DMA_CCR_TCIE;          /* TC interrupt enable    */

    /* Enable the DMA1_Ch5 interrupt in NVIC (IRQ 15 on STM32F303) */
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    NVIC_SetPriority(DMA1_Channel5_IRQn, 2);

    /* NOTE: TXDMAEN is NOT set here — it is toggled on/off in
       SPI_DMA_Start() / DMA ISR to avoid interfering with blocking SPI. */
}

void SPI_DMA_Start(const uint16_t *buf, uint16_t count)
{
    /* Ensure any previous transfer is done */
    while (g_spi_dma_busy) { }

    g_spi_dma_busy = 1;

    DMA1_Channel5->CCR  &= ~DMA_CCR_EN;         /* disable to configure  */
    DMA1_Channel5->CCR  |= DMA_CCR_MINC;        /* ensure MINC is set    */
    DMA1_Channel5->CMAR  = (uint32_t)buf;        /* source address        */
    DMA1_Channel5->CNDTR = count;                /* number of 16-bit xfers*/
    DMA1->IFCR           = DMA_IFCR_CTCIF5       /* clear TC flag         */
                         | DMA_IFCR_CHTIF5
                         | DMA_IFCR_CTEIF5
                         | DMA_IFCR_CGIF5;

    SPI2->CR2 |= SPI_CR2_TXDMAEN;               /* arm DMA requests      */
    DMA1_Channel5->CCR  |= DMA_CCR_EN;           /* go!                   */
}

int SPI_DMA_Busy(void)
{
    return g_spi_dma_busy;
}