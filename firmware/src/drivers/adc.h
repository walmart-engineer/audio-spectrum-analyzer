#ifndef ADC_H
#define ADC_H

#include <stdint.h> 
#include "stm32f303xc.h"
#include "drivers/delay.h"
#include "config.h"

static volatile uint16_t adc_buffer[2 * ADC_BUFFER_SIZE];


void adc_init(void); //initializare ADC
void adc_restart_audio_stream(void);
void adc_start_audio_stream(void);
void measure_vdda(uint32_t* vdda_mv);
uint16_t adc_snapshot(void);


void ADC1_2_ISR(void);

#endif