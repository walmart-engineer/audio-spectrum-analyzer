#ifndef FFT_H
#define FFT_H

#include <stdint.h>
#include "config.h"



static volatile int16_t fft_sample_ring[FFT_SAMPLE_RING_SIZE];

void fft_ring_push(uint16_t x);
void unwind_ring(float* frame);


#endif