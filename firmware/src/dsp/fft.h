#ifndef FFT_H
#define FFT_H

#include <stdint.h>
#include "config.h"



static volatile ring_int16 fft_sample_ring;

void fft_ring_push(uint16_t x);
void unwind_ring(float* frame);


#endif