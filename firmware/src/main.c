#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "system/system.h"  
#include "drivers/gpio.h"
#include "misc/MAX7219.h"

#define MIC_PIN 1u
#define MIC_PORT 'A'



/* ---- main --------------------------------------------------------- */
int main(void)
{
    uint32_t vdda; // VDDA in mV
    SPI_Init();
    MAX7219_Init();

    gpio_set(MIC_PORT, MIC_PIN, ANALOG);

    
    
    adc_init();
    
    measure_vdda(&vdda);

    volatile uint16_t adc_value; 

    uint32_t voltage;

    for(;;){

        adc_value = adc_snapshot();
        voltage = ((adc_value * vdda) >> 12);
        render(voltage);
        delay_ms(100);
    }

}
