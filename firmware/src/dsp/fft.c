#include "fft.h"

ring_int16* ring_addr = &fft_sample_ring;

void fft_ring_push(uint16_t x){

    if(ring_addr->last > 4095u) ring_addr->last = 0u;

    ring_addr->arr[ring_addr->last] = x;
    ++ring_addr->last;

}