#include "ISR/ISR_Handlers.h"

void Init_Handlers(void){
}

void SysTick_Handler(void){
    GPIOE->ODR ^= (1u << 8);
}