#include "button.h"

void button_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* MODER = 00 (input).  The board has an external pull-down, so no
     * internal pull is needed — PUPDR stays 00. */
    BTN_PORT->MODER  &= ~(3u << (BTN_PIN * 2u));
    BTN_PORT->PUPDR  &= ~(3u << (BTN_PIN * 2u));
}

static inline uint32_t button_raw(void)
{
    return (BTN_PORT->IDR >> BTN_PIN) & 1u;   /* 1 = pressed on this board */
}

void button_wait_press(void)
{
    /* Wait for release first, so a button still held from a previous
     * call doesn't fall straight through. */
    while (button_raw()) { }
    delay_ms(20u);

    /* Wait for a press that survives the debounce window. */
    for (;;) {
        while (!button_raw()) { }
        delay_ms(20u);
        if (button_raw()) break;   /* still down: real press, not a bounce */
    }
}