#ifndef MAX7219_H
#define MAX7219_H

#include <stdint.h>
#include "drivers/spi.h"


static inline void MAX7219_Select(void){
    GPIOD->BRR  = (1u << 8);
}
static inline void MAX7219_Deselect(void){
    GPIOD->BSRR = (1u << 8);
}

static void MAX7219_Send(uint8_t addr, uint8_t data){
    MAX7219_Select();                             /* LOAD LOW              */
    (void)SPI_Xfer((uint16_t)(addr << 8) | data);  /* 16-bit frame out     */
    SPI_WaitDone();                               /* wait for shift done   */
    MAX7219_Deselect();                           /* LOAD HIGH -> latches  */
}

static void MAX7219_Init(void){
    MAX7219_Send(0x0F, 0x00);   /* Display Test  = normal operation      */
    MAX7219_Send(0x0B, 0x07);   /* Scan Limit    = all 8 digits (0..7)   */
    MAX7219_Send(0x09, 0xFF);   /* Decode Mode   = BCD decode all digits */
    MAX7219_Send(0x0A, 0x07);   /* Intensity     = 8/16 brightness       */
    MAX7219_Send(0x0C, 0x01);   /* Shutdown reg  = normal operation (ON) */

    /* blank all 8 digits to start clean */
    for (uint8_t d = 1; d <= 8; ++d)
        MAX7219_Send(d, 0x0F);  /* 0x0F = blank in BCD decode mode       */
}

void render(uint32_t number){

    SPI_Init();          /* configures GPIOs + SPI2 peripheral */
    MAX7219_Init();      /* configures the display chip        */

    uint8_t digits[8] = {0};

    if(number > 99999999U)
        number = 99999999U;
    
    for(uint8_t i = 0; i < 8; ++i){
        digits[i] = number % 10;
        number /= 10;
    }

    for(uint8_t j = 0; j < 8; ++j){
        MAX7219_Send(j + 1, digits[j]);
    }

}


#endif