#include "adc.h"
#include "dma.h"

#define VREFINT_CAL_ADDR ((uint16_t *)0x1FFFF7BAu)
#define VREFINT_CH 18u

#define MIC_ADC_CH 2u 

static volatile uint32_t g_adc_overruns;

void adc_init(void)
{
    delay_init();

    // ---- ADC Bring up ----

    RCC->AHBENR |= RCC_AHBENR_ADC12EN;

    ADC1_2_COMMON->CCR |= ADC12_CCR_CKMODE_0 | ADC12_CCR_VREFEN; // Clock sincronizat cu magistrala AHB1, prescaler 1
    
    ADC1->CR &= ~ADC_CR_ADVREGEN;
    ADC1->CR |=  ADC_CR_ADVREGEN_0;
    delay_us(20u);

    ADC1->CR &= ~ADC_CR_ADCALDIF;
    ADC1->CR |=  ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL) { }   
    for (volatile int i = 0; i < 8; ++i) { }  
    
    ADC1->ISR = ADC_ISR_ADRDY;
    ADC1->CR  |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) { }

    ADC1->SMPR1 |= (7u << (3u * MIC_ADC_CH));
    ADC1->SQR1  =  (MIC_ADC_CH << 6u);
    
    ADC1->IER = ADC_IER_OVRIE;
    NVIC_EnableIRQ(ADC1_2_IRQn);
    ADC1->CFGR = 0u;
}


uint16_t adc_snapshot(void){
    
    ADC1->ISR |= ADC_ISR_EOC;
    ADC1->CR = ADC_CR_ADVREGEN_0 | ADC_CR_ADSTART;
    while(!(ADC1->ISR & ADC_ISR_EOC)){}
    return ADC1->DR;

}

void adc_start_audio_stream(void){

    ADC1->CFGR = (0u <<  3)   /* RES    = 00 -> 12-bit                    */
           | (0u <<  5)   /* ALIGN  = 0  -> right aligned             */
           | (11u << 6)   /* EXTSEL = 1011 = EXT11 = TIM2_TRGO        */
           | (1u << 10)   /* EXTEN  = 01 -> rising edge               */
           | (1u << 12)   /* OVRMOD = 1  -> overwrite on overrun      */
           | (0u <<  1)   /* DMACFG = 1  -> DMA circular mode         */
           | (1u <<  0);  /* DMAEN  = 1                               */
                          /* CONT stays 0: one conversion per trigger */

    for(uint8_t i = 0; i < 8; ++i) { } // Little delay

    DMA1_Channel1->CCR |= DMA_CCR_EN;                 /* arm the DMA...           */
    ADC1->CR = ADC_CR_ADVREGEN_0 | ADC_CR_ADSTART;    /* ...then arm the ADC...   */
    TIM2->CR1 |= TIM_CR1_CEN;                         /* start the timer          */  
}



void adc_restart_audio_stream(void){
    /* 1. Stop the ADC's regular conversions. ADSTP is only writable
          while ADSTART = 1, so guard it. Hardware clears both bits
          when the abort completes. */
    if (ADC1->CR & ADC_CR_ADSTART) {
        ADC1->CR = ADC_CR_ADVREGEN_0 | ADC_CR_ADSTP;
        while (ADC1->CR & ADC_CR_ADSTART) { }
    }

    /* 2. Re-arm the DMA. Disabling and re-enabling is what resets
          CNDTR and the internal memory pointer back to the start
          of the buffer -- this is the step that restores phase. */
    dma_adc_rearm();

    /* 3. Clear the ADC's flags and start again. */
    ADC1->ISR = ADC_ISR_OVR | ADC_ISR_EOC | ADC_ISR_EOS;
    ADC1->CR  = ADC_CR_ADVREGEN_0 | ADC_CR_ADSTART;
}




void measure_vdda(uint32_t* vdda_mv){

    const uint32_t cfgr_save  = ADC1->CFGR;
    const uint32_t smpr2_save = ADC1->SMPR2;
    const uint32_t sqr1_save  = ADC1->SQR1;

    if (ADC1->CR & ADC_CR_ADSTART) {          /* stop a running stream */
        ADC1->CR = ADC_CR_ADVREGEN_0 | ADC_CR_ADSTP;
        while (ADC1->CR & ADC_CR_ADSTART) { }
    }

    ADC1->CFGR  = 0u;
    ADC1->SMPR2 = (smpr2_save & ~(7u << 24)) | (6u << 24);
    ADC1->SQR1  = (VREFINT_CH << 6u);

    ADC1->ISR = ADC_ISR_EOC;
    ADC1->CR  = ADC_CR_ADVREGEN_0 | ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC)){}
    uint16_t raw = ADC1->DR;
    uint16_t cal = (uint16_t)(*VREFINT_CAL_ADDR);
    *vdda_mv = (3300u * cal) / raw;

    ADC1->SMPR2 = smpr2_save;
    ADC1->SQR1  = sqr1_save;
    ADC1->CFGR  = cfgr_save;
}


void ADC1_2_ISR(void){

    if (ADC1->ISR & ADC_ISR_OVR) {
        ADC1->ISR = ADC_ISR_OVR;      /* write 1 to clear             */
        g_adc_overruns++;             /* surface this on the display  */
        adc_restart_audio_stream();         /* see below                    */
    }
}