#ifndef GPIO_H
#define GPIO_H

#define HIGH 1u
#define LOW 0u
#define ANALOG 3u
#define DIGITAL_OUT 1u
#define DIGITAL_IN 0u

#include <stdint.h>
#include "stm32f303xc.h"

uint8_t gpio_set(uint8_t port, uint8_t pin_number, uint8_t pin_mode); 
uint8_t gpio_write(uint8_t port, uint8_t pin_number, uint8_t state);
uint8_t gpio_digital_read(uint8_t port, uint8_t pin_number);

#endif