#include <stdint.h>
#include "stm32f303xc.h"
#include "config.h"
#include "drivers/delay.h"

static volatile uint16_t dac_buffer[DAC_BUFFER_SIZE];

void dac_init(void);
inline void dac_write(uint16_t value);
void dac_push(uint16_t x);
