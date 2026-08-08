#ifndef MAX7219_H
#define MAX7219_H

#include <stdint.h>
#include "drivers/spi.h"

static inline void MAX7219_Select(void);
static inline void MAX7219_Deselect(void);

void MAX7219_Send(uint8_t addr, uint8_t data);

void MAX7219_Init(void);

void render(uint32_t number);

#endif