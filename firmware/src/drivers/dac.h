#include <stdint.h>
#include "stm32f303xc.h"


void dac_init(void);

inline void dac_write(uint16_t value);