#include "stm32f303xc.h"
#include "drivers/tim3.h"

int main(void)
{
    /* --- 1. Enable the AHB clock to GPIO port E ---------------------
     * Every peripheral on STM32 is clock-gated off at reset. Writing
     * its registers before enabling the clock does nothing (reads
     * return 0) — no fault, no warning, just silence.                 */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* Read the register back: guarantees the write has landed on the
     * peripheral bus before we touch GPIOE (see RM0316 & F3 errata on
     * clock-enable delay).                                            */
    (void)RCC->AHBENR;

    /* --- 2. Configure PE8 as a general-purpose output ---------------
     * MODER has 2 bits per pin: 00 input, 01 output, 10 alt-func,
     * 11 analog. Port E resets to all-zeros (all inputs), so OR-ing
     * in the '01' pattern for pin 8 is sufficient and touches nothing
     * else.                                                           */
    GPIOA->MODER |= (0x1u << (5 * 2));

    /* Defaults are already what we want: push-pull, low speed, no
     * pull resistors. Fine for an LED.                                */

    /* --- 3. Blink forever -------------------------------------------
     * BSRR: writing bit n sets pin n; writing bit n+16 resets it.
     * Unlike ODR ^= x (read-modify-write), BSRR is a pure write —
     * atomic, interrupt-safe, and the habit worth building now for
     * when interrupts exist.                                          */
    (void)delay_init();

     for (;;) {
        GPIOA->BSRR = (1u << 5);         
        delay_us(20);
        GPIOA->BSRR = (1u << (5 + 16));   
        delay_us(20);
    }
}
