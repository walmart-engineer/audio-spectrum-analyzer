#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "system/system.h"  
#include "drivers/gpio.h"

#define MIC_PIN 1u
#define MIC_PORT 'A'

static uint32_t vdda; // VDDA in mV

/* ---- main --------------------------------------------------------- */
int main(void)
{

    // volatile uint16_t adc_value;

    // delay_init();

    adc_init();

    measure_vdda(&vdda);

    (void)gpio_set('E', 10u, DIGITAL_OUT);

    for(;;){
        gpio_write('E', 10u, HIGH);
        delay_ms(100);
        gpio_write('E', 10u, LOW);
        delay_ms(100);
    }
}
