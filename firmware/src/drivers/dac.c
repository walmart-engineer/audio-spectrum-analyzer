#include "dac.h"

void dac_init(void){

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    (void)RCC->AHBENR;

    RCC->APB1ENR |= RCC_APB1ENR_DAC1EN;

    
    GPIOA->MODER &= ~(3 << 8);
    GPIOA->MODER |=  (3 << 8);
    


    DAC->CR |=  DAC_CR_EN1;
    DAC->CR &=~ DAC_CR_BOFF1;
    DAC->CR |=  DAC_CR_TEN1;
    DAC->CR &= ~(7 << 3);
    DAC->CR |=  (4 << 3);


}

void dac_write(uint16_t value){
    DAC->DHR12R1 =value;
}