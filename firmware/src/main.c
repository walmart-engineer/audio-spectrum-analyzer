/*
 * main.c — register-level blinky for the STM32F3 Discovery
 *
 * Blinks LD4 (blue LED, pin PE8) using only the reset-default 8 MHz HSI
 * clock. Three register operations, total:
 *   1. RCC->AHBENR   : give GPIO port E a clock (peripherals are dead
 *                      until clocked — the #1 beginner trap on STM32)
 *   2. GPIOE->MODER  : make PE8 an output
 *   3. GPIOE->BSRR   : set / reset the pin atomically
 */

#include "stm32f303xc.h"

/* Crude busy-wait. Volatile stops the optimizer deleting the loop.
 * At 8 MHz, ~400k iterations is a visible fraction of a second.
 * (Replaced by a SysTick-based delay the moment we care about time.) */
static void delay(volatile uint32_t count)
{
    while (count--) {
        __NOP();
    }
}

int main(void)
{
    /* --- 1. Enable the AHB clock to GPIO port E ---------------------
     * Every peripheral on STM32 is clock-gated off at reset. Writing
     * its registers before enabling the clock does nothing (reads
     * return 0) — no fault, no warning, just silence.                 */
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    /* Read the register back: guarantees the write has landed on the
     * peripheral bus before we touch GPIOE (see RM0316 & F3 errata on
     * clock-enable delay).                                            */
    (void)RCC->AHBENR;

    /* --- 2. Configure PE8 as a general-purpose output ---------------
     * MODER has 2 bits per pin: 00 input, 01 output, 10 alt-func,
     * 11 analog. Port E resets to all-zeros (all inputs), so OR-ing
     * in the '01' pattern for pin 8 is sufficient and touches nothing
     * else.                                                           */
    GPIOE->MODER |= (0x1u << (8 * 2));

    /* Defaults are already what we want: push-pull, low speed, no
     * pull resistors. Fine for an LED.                                */

    /* --- 3. Blink forever -------------------------------------------
     * BSRR: writing bit n sets pin n; writing bit n+16 resets it.
     * Unlike ODR ^= x (read-modify-write), BSRR is a pure write —
     * atomic, interrupt-safe, and the habit worth building now for
     * when interrupts exist.                                          */
    for (;;) {
        GPIOE->BSRR = (1u << 8);          /* LD4 on  */
        delay(400000);
        GPIOE->BSRR = (1u << (8 + 16));   /* LD4 off */
        delay(400000);
    }
}
