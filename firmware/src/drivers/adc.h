#ifndef ADC_H
#define ADC_H

#include <stdint.h> 
#include "stm32f303xc.h"
#include "drivers/delay.h"

void adc_init(void); //initializare ADC + configurare
void adc_restart_audio_stream(void);
void adc_start_audio_stream(void);
void measure_vdda(uint32_t* vdda_mv);


void ADC1_2_ISR(void);

#endif