#include "stm32f303xc.h"
#include "tim2.h"

#define TIMER_CLOCK_HZ  8000000u      /* -> 72000000u after the PLL milestone */
#define TICK_HZ         10000u        /* 100 us per count                     */
#define PSC_VALUE       ((TIMER_CLOCK_HZ / TICK_HZ) - 1u)
#define TICKS_PER_MS    (TICK_HZ / 1000u)

void delay_init_tim2(void){

    // clock enable for timer 2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR;
    // reseting the counter register

    TIM2->CR1 = TIM_CR1_OPM | TIM_CR1_URS;   

    // setting the prescaler so that CLK has 7.812 kHz
    TIM2->PSC = PSC_VALUE;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = ~TIM_SR_UIF;

}

void delay_ms(uint32_t milis){


    // clipping
    if (milis < 1u)      { milis = 1u; }        
    if (milis > 10000u)  { milis = 10000u; }

    TIM2->ARR = (milis * TICKS_PER_MS) - 1u;
    TIM2->CNT = 0u;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = ~TIM_SR_UIF;
    TIM2->CR1 |= TIM_CR1_CEN;

    while ((TIM2->SR & TIM_SR_UIF) == 0u) { }

    TIM2->SR = ~TIM_SR_UIF;
}
