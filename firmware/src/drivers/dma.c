#include "dma.h"

#define BUFFER_SIZE 512

static volatile uint16_t adc_buffer[2 * BUFFER_SIZE];

void dma_adc_init(void){
    DMA1_Channel1->CCR   = 0u;
    DMA1->IFCR           = DMA_IFCR_CGIF1;
    DMA1_Channel1->CPAR  = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR  = (uint32_t)adc_buffer;
    DMA1_Channel1->CNDTR = 2u * BUFFER_SIZE;
    DMA1_Channel1->CCR   = (1u<<10) 
                         | (1u<<8)       /* 16-bit both sides */
                         | DMA_CCR_MINC 
                         | DMA_CCR_CIRC
                         | DMA_CCR_HTIE 
                         | DMA_CCR_TCIE 
                         | DMA_CCR_TEIE
                         | (2u << 12);              /* PL = high         */

    
    NVIC_SetPriority(DMA1_Channel1_IRQn, 0u);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void dma_adc_rearm(void){
    DMA1_Channel1->CCR  &= ~DMA_CCR_EN;
    DMA1->IFCR           = DMA_IFCR_CGIF1;
    DMA1_Channel1->CPAR  = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR  = (uint32_t)adc_buffer;
    DMA1_Channel1->CNDTR = 2u * BUFFER_SIZE;
    DMA1_Channel1->CCR  |= DMA_CCR_EN;
}

void DMA1_Channel1_ISR(void){

    const uint32_t flags = DMA1->ISR;

    if (flags & DMA_ISR_TEIF1) {          /* bus error: the stream is dead */
        DMA1->IFCR = DMA_IFCR_CGIF1;
        generic_fault_handler();
        return;
    }
    if (flags & DMA_ISR_HTIF1) {
        DMA1->IFCR = DMA_IFCR_CHTIF1;
        decimate_block((const uint16_t *)&adc_buffer[0]);
    }
    if (flags & DMA_ISR_TCIF1) {
        DMA1->IFCR = DMA_IFCR_CTCIF1;
        decimate_block((const uint16_t *)&adc_buffer[BUFFER_SIZE]);
    }
}