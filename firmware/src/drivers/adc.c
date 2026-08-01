#include "adc.h"
#include "stm32f3xx.h"
#include "delay.h"

#define MIC_CH 2u //Adauga canalul folosit PA1, canal 2

void ADC_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_ADC12EN;
    (void)RCC->AHBENR;

    GPIOA->MODER  |=  (3u << (1u * 2u));
    GPIOA->PUPDR  &= ~(3u << (1u * 2u));

    ADC1_2_COMMON->CCR |= ADC12_CCR_CKMODE_0;

    ADC1->CR &= ~ADC_CR_ADVREGEN;
    ADC1->CR |=  ADC_CR_ADVREGEN_0;
    delay_us(20u);

    ADC1->CR &= ~ADC_CR_ADCALDIF;
    ADC1->CR |=  ADC_CR_ADCAL;
    {
        uint32_t t = 1000000u;
        while ((ADC1->CR & ADC_CR_ADCAL) && t--) { }
        if (t == 0u) return; 
    }

    ADC1->ISR  = ADC_ISR_ADRDY;
    ADC1->CR  |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) { }

    ADC1->SMPR1 |= (7u << (3u * MIC_CH));
    ADC1->SQR1  =  (MIC_CH << 6);
}

uint16_t ADC_Read(void){
    ADC1->CR |= ADC_CR_ADSTART;
    while(!(ADC1->ISR & ADC_ISR_EOC)) { }
    return (uint16_t)ADC1->DR;
}