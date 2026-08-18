#ifndef DECIMATE_H
#define DECIMATE_H

#include <stdint.h>
#include "config.h"
#include "dsp/fft.h"
#include "drivers/dac.h"

void decimate_block(const uint16_t* raw);

#endif