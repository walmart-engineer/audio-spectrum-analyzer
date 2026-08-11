#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stddef.h>

#include "stm32f303xc.h"


void SPI_Init(void);

uint16_t SPI_Xfer(uint16_t out);

void SPI_Write(const uint16_t *p, size_t n);

void SPI_WaitDone(void);

/* ---- data-size switching (ILI9341 needs 8-bit cmds, 16-bit pixels) ---- */
void SPI_SetDataSize8(void);
void SPI_SetDataSize16(void);

/* ---- transmit-only helpers (no read-back, drains RX to avoid OVR) ----- */
void SPI_Tx8(uint8_t b);
void SPI_Tx16(uint16_t w);

/* ---- DMA-driven bulk transfer (SPI2_TX → DMA1_Channel5) --------------- */
void SPI_DMA_Init(void);
void SPI_DMA_Start(const uint16_t *buf, uint16_t count);
int  SPI_DMA_Busy(void);

#endif