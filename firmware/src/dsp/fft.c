#include "fft.h"

static volatile uint16_t last;

void fft_ring_push(int16_t x){

    if(last > FFT_SAMPLE_RING_SIZE - 1) last = 0u;

    fft_sample_ring[last] = x;
    ++last;

}