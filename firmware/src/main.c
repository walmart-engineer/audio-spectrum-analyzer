#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "system/system.h"  



int main(void)
{
    volatile uint8_t status = SysTick_Config(SYS_CLK/1000000U);
    uint32_t current, prev;

    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;                                          
    (void)RCC->AHBENR;

    GPIOE->MODER |= (0x1u << (9 * 2));

    for (;;) {
        current = millis();
        if(current - prev > 99U){
            GPIOE->ODR ^= (1u << 9);
            prev = current;
        }
    }
}
