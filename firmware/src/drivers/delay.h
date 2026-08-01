#ifndef TIM3_H
#define TIM3_H
#include <stdint.h>
#include "stm32f303xc.h"

void delay_init(); // inits timer for microseconds
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);

#endif