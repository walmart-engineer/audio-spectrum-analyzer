#include "decimate.h"

volatile uint32_t ready_128 = 0u;

void decimate_block(const uint16_t *raw)
{
    for (unsigned i = 0u; i < ADC_BUFFER_SIZE; i += 4u) {
        uint32_t sum = (uint32_t)raw[i]      + (uint32_t)raw[i + 1u]
                     + (uint32_t)raw[i + 2u] + (uint32_t)raw[i + 3u];
                                  /* 0 .. 16380  -- 14 bits          */

        uint16_t us = (uint16_t)(sum >> 2); 

        int16_t s = (int16_t)((sum >> 1) - 4096);
                                  /* >>1 keeps 13 bits, -4096 centres */
        fft_ring_push(s);             /* the FFT's 4096-sample history    */
        ready_128++;
        dac_push(us);              /* pass-through, 4x   */
    }
}