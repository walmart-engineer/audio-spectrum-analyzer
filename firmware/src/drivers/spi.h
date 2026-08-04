#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stddef.h>

#include "stm32f303xc.h"


void SPI_Init(void);

uint16_t SPI_Xfer(uint16_t out);

void SPI_Write(const uint16_t *p, size_t n);

void SPI_WaitDone(void);
#endif