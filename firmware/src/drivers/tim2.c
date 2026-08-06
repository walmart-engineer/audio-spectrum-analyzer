#include "tim2.h"

void timer2_init(void){

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR;


    TIM2->PSC = 0u;
    TIM2->ARR = 375u - 1u;            /* 72 MHz / 192 KHz = 375        */  
    TIM2->CR2 = (2u << 4);            /* MMS = TRGO on update          */
    TIM2->CR1 = TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;           /* load shadows — see note below */
    TIM2->SR  = 0u;
}
