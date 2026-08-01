/*
    Timer 2 este folosit ca si trigger pentru ADC
    si DAC. Ele impart acelasi clock cycle pentru
    a nu se desincroniza.
*/

#ifndef TIM2_H
#define TIM2_H

#include "stm32f303xc.h"
#include <stdint.h>

void timer2_init(void);

#endif