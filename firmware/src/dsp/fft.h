#ifndef FFT_H
#define FFT_H

#include <stdint.h>
#include "config.h"
#include "twiddle.h"
#include "window.h"

extern volatile int16_t fft_sample_ring[FFT_SAMPLE_RING_SIZE];
extern volatile float fft_frame_re[FFT_FRAME_SIZE / 2];
extern volatile float fft_frame_im[FFT_FRAME_SIZE / 2];
extern volatile uint8_t hop_ready;
extern volatile uint16_t start_index;

void fft_ring_push(int16_t x);
void frame_prepare(const int16_t *ring, uint32_t write_idx,
                   float *re, float *im);
void wait_for_ring_init();
void fft_radix2(float* re, float* im, uint16_t n);


#endif