#ifndef ADC_H
#define ADC_H

#include <stdint.h> 
#include "stm32f303xc.h"
#include "drivers/delay.h"

void ADC_Init(void); //initializare ADC + configurare
uint16_t ADC_Read(void); //returneaza o conversie

#endif