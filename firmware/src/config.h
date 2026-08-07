#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

static const uint16_t ADC_BUFFER_SIZE = 512u;
static const uint16_t DAC_BUFFER_SIZE = 2048u;
static const uint16_t FFT_SAMPLE_RING_SIZE = 4096u;
static const uint16_t FFT_FRAME = 4096u;

typedef struct ring_int16 {
    uint16_t arr[FFT_SAMPLE_RING_SIZE];
    uint16_t last;
} ring_int16;

#endif