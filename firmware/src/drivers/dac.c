#include "dac.h"

volatile uint16_t dac_buffer[DAC_BUFFER_SIZE]; // buffer comun folosit de procesare si DAC DMA
static volatile uint16_t last = 0; // for the ring of the dac

void dac_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    (void)RCC->AHBENR;

    gpio_set('A', 4u, ANALOG);

    RCC->APB1ENR |= RCC_APB1ENR_DAC1EN; //Activeaza CLK DAC
    (void)RCC->APB1ENR;

    DAC->CR &= ~(DAC_CR_EN1
               | DAC_CR_BOFF1
               | DAC_CR_TEN1
               | DAC_CR_TSEL1
               | DAC_CR_WAVE1
               | DAC_CR_DMAEN1
               | DAC_CR_DMAUDRIE1);

    for (uint32_t i = 0u; i < DAC_BUFFER_SIZE; ++i) { //Umple uffer cu valoarea de mijloc
        dac_buffer[i] = 2048u;
    }

    DAC->DHR12R1 = 2048u;

    DAC->CR |= (4u << DAC_CR_TSEL1_Pos) //selecteaza TIM2 trept trigger
             | DAC_CR_TEN1 
             | DAC_CR_DMAEN1 //permite cererile DMA
             | DAC_CR_EN1; //activeaza canalul 1 DAC
}

void dac_write(uint16_t value){
    DAC->DHR12R1 = value;
}

void dac_push(uint16_t x){
    for(uint8_t i = 0; i < 4; ++i){
        dac_buffer[last] = x;
        last++; // nu se poate optimiza pentru ca sare peste valorile 0 si 1 

        if(last >= DAC_BUFFER_SIZE) {
            last = 0u;
        }
    }
}