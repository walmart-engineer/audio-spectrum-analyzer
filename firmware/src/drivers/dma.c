#include "dma.h"



void dma_adc_init(void){
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    (void)RCC->AHBENR;
    //TURNS OFF THE DMA
    DMA1_Channel1->CCR   = 0u;
    //RESETS THE FLAGS
    DMA1->IFCR           = DMA_IFCR_CGIF1;
    //TELLS THE DMA WHERE TO READ FROM
    DMA1_Channel1->CPAR  = (uint32_t)&ADC1->DR;
    //TELLS THE DMA WHERE TO WRITE
    DMA1_Channel1->CMAR  = (uint32_t)adc_buffer;
    //NUMBER OF VALUES TO TRANSFER
    DMA1_Channel1->CNDTR = 2u * ADC_BUFFER_SIZE;
    //SETS THE DMA
    DMA1_Channel1->CCR   = (1u<<10) 
                         | (1u<<8)       /* 16-bit both sides */
                         | DMA_CCR_MINC 
                         | DMA_CCR_CIRC
                         | DMA_CCR_HTIE 
                         | DMA_CCR_TCIE 
                         | DMA_CCR_TEIE
                         | (2u << 12);              /* PL = high         */

    //SETS PRIORITY TO LEVEL 0
    NVIC_SetPriority(DMA1_Channel1_IRQn, 0u);
    //ENABLES THE INTERRUPT
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void dma_adc_rearm(void){
    //RECONFIGURES THE DMA
    DMA1_Channel1->CCR  &= ~DMA_CCR_EN;
    DMA1->IFCR           = DMA_IFCR_CGIF1;
    DMA1_Channel1->CPAR  = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR  = (uint32_t)adc_buffer;
    DMA1_Channel1->CNDTR = 2u * ADC_BUFFER_SIZE;
    DMA1_Channel1->CCR  |= DMA_CCR_EN;
}

void DMA1_Channel1_ISR(void){

    const uint32_t flags = DMA1->ISR;
    /*CHECKS FOR ERRORS AND RESETS THE FLAG
    IF ERRORS FOUND */
    if (flags & DMA_ISR_TEIF1) {          /* bus error: the stream is dead */
        DMA1->IFCR = DMA_IFCR_CGIF1;
        generic_fault_handler();
        return;
    }
    /*CHECKS IF THE TRANSFER IS HALF DONE 
    AND CLEARS THE HALF INTERRUPT FLAG*/
    if (flags & DMA_ISR_HTIF1) {
        DMA1->IFCR = DMA_IFCR_CHTIF1;
        gata = 1;
        return;
        //decimate_block((const uint16_t *)&adc_buffer[0]);
    }
    /*CHECKS IF THE TRANSFER IS FULLY DONE AND
    CLEARS THE COMPLETE TRANSFER FLAG*/
    if (flags & DMA_ISR_TCIF1) {
        DMA1->IFCR = DMA_IFCR_CTCIF1;
        gata = 0;
        //decimate_block((const uint16_t *)&adc_buffer[ADC_BUFFER_SIZE]);
    }
}