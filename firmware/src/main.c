
#include "stm32f303xc.h"
#include "adc.h"
#include "delay.h"  

#define NR_ESANTIOANE 64


volatile uint16_t buffer[NR_ESANTIOANE];
volatile uint16_t index = 0;

int main(void)
{
    /* --- 1. Enable the AHB clock to GPIO port E ---------------------
     * Every peripheral on STM32 is clock-gated off at reset. Writing
     * its registers before enabling the clock does nothing (reads
     * return 0) — no fault, no warning, just silence.                 */
     RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
     RCC->AHBENR |= RCC_AHBENR_GPIOEEN;


    /* --- 2. Configure PE8 as a general-purpose output ---------------
     * MODER has 2 bits per pin: 00 input, 01 output, 10 alt-func,
     * 11 analog. Port E resets to all-zeros (all inputs), so OR-ing
     * in the '01' pattern for pin 8 is sufficient and touches nothing
     * else.                                                           */
    GPIOE->MODER |= (0x3u << (8 * 2));
    GPIOE->MODER |= (0x1u << (1 * 2)); //seteaza pinul PA1 ca intrare analog


     ADC_Init();

    /* --- 3. Blink forever -------------------------------------------
     * BSRR: writing bit n sets pin n; writing bit n+16 resets it.
     * Unlike ODR ^= x (read-modify-write), BSRR is a pure write —
     * atomic, interrupt-safe, and the habit worth building now for
     * when interrupts exist.                                          */
    for (;;) {
        buffer[index] = ADC_Read();
        index = (index + 1) % NR_ESANTIOANE;

    }
}
