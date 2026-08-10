#include "fft.h"

static volatile uint16_t last = 0u;

void fft_ring_push(int16_t x){

    if(last > 4095u) last = 0u;

    fft_sample_ring[last] = x;
    ++last;

}