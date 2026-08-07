#include "dac.h"

static volatile uint16_t last; // for the ring of the dac

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

inline void dac_write(uint16_t value){
    DAC->DHR12R1 =value;
}

void dac_push(uint16_t x){
    
    if(last > DAC_BUFFER_SIZE - 1) last = 0u;

    dac_buffer[last] = x;
    ++last;


}