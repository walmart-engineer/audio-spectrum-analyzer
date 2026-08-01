#include "delay.h"
#include "stm32f303xc.h"

void delay_us(uint32_t us){
    while(us--){
        for( volatile uint32_t i = 0; i < 8; ++i){
            __NOP();
        }
    }
}

void delay_ms(uint32_t ms){
    while(ms--){
       delay_us(1000u);
    }
}