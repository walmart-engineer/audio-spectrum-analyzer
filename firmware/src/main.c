
#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"  

#define NR_ESANTIOANE 64
#define SYS_CLK 8000000U

uint16_t fact(uint8_t val);

int main(void)
{

    /*ADC Circular Sampling*/

    // volatile uint16_t buffer[NR_ESANTIOANE];
    // volatile uint16_t index = 0;
    // /* --- 1. Enable the AHB clock to GPIO port E ---------------------
    //  * Every peripheral on STM32 is clock-gated off at reset. Writing
    //  * its registers before enabling the clock does nothing (reads
    //  * return 0) — no fault, no warning, just silence.                 */
    //  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    //  RCC->AHBENR |= RCC_AHBENR_GPIOEEN;


    // /* --- 2. Configure PE8 as a general-purpose output ---------------
    //  * MODER has 2 bits per pin: 00 input, 01 output, 10 alt-func,
    //  * 11 analog. Port E resets to all-zeros (all inputs), so OR-ing
    //  * in the '01' pattern for pin 8 is sufficient and touches nothing
    //  * else.                                                           */
    // GPIOE->MODER |= (0x3u << (8 * 2));
    // GPIOE->MODER |= (0x1u << (1 * 2)); //seteaza pinul PA1 ca intrare analog

    // delay_init();

    // ADC_Init();

    
    // for (;;) {
    //     buffer[index] = ADC_Read();
    //     index = (index + 1) % NR_ESANTIOANE;

    // }

    /* Blinky */

    volatile uint8_t status = SysTick_Config(SYS_CLK/2U);
    
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;                                          
    (void)RCC->AHBENR;

    GPIOE->MODER |= (0x1u << (8 * 2)) | (0x1u << (9 * 2));

    for (;;) {
        GPIOE->BSRR = (1u << 9);          
        delay_ms(1000);
        GPIOE->BSRR = (1u << (9 + 16));   
        delay_ms(1000);
    }

}


// uint16_t fact(uint8_t val){
//     if(val == 1) return 1;
//     else{
//         return val * fact(val - 1);
//     }
// }