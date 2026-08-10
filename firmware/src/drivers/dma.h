#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include "stm32f303xc.h"
#include "system/system.h"
#include "dsp/decimate.h"
#include "config.h"
#include "adc.h"

static volatile uint8_t gata;

static volatile uint32_t prev_cycles;
static volatile uint32_t current_cycles;

void dma_adc_init(void);
void dma_dac_init(void);
void dma_spi_init(void);
void dma_adc_rearm(void);

void DMA1_Channel1_ISR(void);

#endif