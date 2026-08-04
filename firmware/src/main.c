
#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "drivers/spi.h"

/* ---- MAX7219 helpers (inline in main.c) --------------------------- */

static inline void MAX7219_Select(void)   { GPIOD->BRR  = (1u << 8); }
static inline void MAX7219_Deselect(void) { GPIOD->BSRR = (1u << 8); }

static void MAX7219_Send(uint8_t addr, uint8_t data)
{
    MAX7219_Select();                             /* LOAD LOW              */
    (void)SPI_Xfer((uint16_t)(addr << 8) | data);  /* 16-bit frame out     */
    SPI_WaitDone();                               /* wait for shift done   */
    MAX7219_Deselect();                           /* LOAD HIGH -> latches  */
}

static void MAX7219_Init(void)
{
    MAX7219_Send(0x0F, 0x00);   /* Display Test  = normal operation      */
    MAX7219_Send(0x0B, 0x07);   /* Scan Limit    = all 8 digits (0..7)   */
    MAX7219_Send(0x09, 0xFF);   /* Decode Mode   = BCD decode all digits */
    MAX7219_Send(0x0A, 0x07);   /* Intensity     = 8/16 brightness       */
    MAX7219_Send(0x0C, 0x01);   /* Shutdown reg  = normal operation (ON) */

    /* blank all 8 digits to start clean */
    for (uint8_t d = 1; d <= 8; ++d)
        MAX7219_Send(d, 0x0F);  /* 0x0F = blank in BCD decode mode       */
}

static void MAX7219_SetDigit(uint8_t digit, uint8_t value)
{
    MAX7219_Send(digit, value);
}

/* ---- main --------------------------------------------------------- */
int main(void)
{

    uint16_t counter = 0, addr = 0, i[9] = {0};

    SPI_Init();          /* configures GPIOs + SPI2 peripheral */
    MAX7219_Init();      /* configures the display chip        */

    /* --- quick test: show "12345678" on the display --- */
    while(counter < 50000U){

        for(uint16_t j = 1; j < 9; ++j){
            if(i[j] > 9){
                i[j] = 0;
                i[j + 1]++;
            }
            MAX7219_SetDigit(j, i[j]);
        }

        delay_ms(10);
        ++i[1];
        ++counter;
    }
}
