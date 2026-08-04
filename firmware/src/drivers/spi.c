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
              | (1u << 3);                /* BR = 001 -> PCLK / 4      */
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
    while (SPI2->SR & SPI_SR_FRLVL)       /* 3. discard RX leftovers */
        { (void)SPI_Get(); }
}