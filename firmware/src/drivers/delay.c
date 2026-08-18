#include "delay.h"

/*
    The Timer Clock is derived from the APB1, but it has a special effect:
        When the APB1 Clock is divided (RCC->CFGR |= RCC_CFGR_PPRE1_DIV2),
        the timer clock gets multiplied by 2. This mechanism offers hardware
        compensation for the APB1 shortcomings in maximum frequency.

    Ex.     APB1_CLK = 32 MHz   ----->    TimerX_CLK = 64 MHz     (x = 2, 3, 4, 5, 6, 7)

*/

#define TIMER_CLOCK_HZ     64000000u
#define TICK_HZ_MS         1000u        /* 100 us per count */    
#define TICK_HZ_US         1000000u                 
#define PSC_VALUE_MS       ((TIMER_CLOCK_HZ / TICK_HZ_MS) - 1u)
#define PSC_VALUE_US       ((TIMER_CLOCK_HZ / TICK_HZ_US) - 1u)

void delay_init(){
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;

    TIM3->CR1 = TIM_CR1_OPM | TIM_CR1_URS; 

    TIM3->PSC = PSC_VALUE_US;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = ~TIM_SR_UIF;
}


void delay_ms(uint16_t millis){

    // clock enable for timer 3
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;
    // reseting the counter register

    TIM3->CR1 = TIM_CR1_OPM | TIM_CR1_URS;   
    TIM3->PSC = PSC_VALUE_MS;

    if (millis < 1u)      { millis = 1u; }        
    if (millis > 10000u)  { millis = 10000u; }

    TIM3->ARR = (uint32_t)millis - 1u;
    TIM3->CNT = 0u;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = ~TIM_SR_UIF;
    TIM3->CR1 |= TIM_CR1_CEN;

    while ((TIM3->SR & TIM_SR_UIF) == 0u) { }

    TIM3->SR = ~TIM_SR_UIF;

}

// functia nu este accurate deloc dar este buna pt un delay in care 
// nu conteaza atat de mult acuratetea

void delay_us(uint16_t micros){

    TIM3->ARR = (uint16_t)micros - 1u;
    TIM3->CNT = 0u;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = ~TIM_SR_UIF;
    TIM3->CR1 |= TIM_CR1_CEN;

    while ((TIM3->SR & TIM_SR_UIF) == 0u) { }

    TIM3->SR = ~TIM_SR_UIF;

}