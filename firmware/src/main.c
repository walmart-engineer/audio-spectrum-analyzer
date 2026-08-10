#include "stm32f303xc.h"
#include "core_cm4.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "system/system.h"  
#include "drivers/gpio.h"
#include "misc/MAX7219.h"
#include "drivers/dma.h"
#include "drivers/tim2.h"

int main(void)
{
    // DWT->CYCCNT = 0;
    // DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    // if (DWT->CYCCNT == 0) {while(1){}}

    uint32_t volatile time_cycles;
    gpio_set('A',0, DIGITAL_IN);
    gpio_set('A',1,ANALOG);
    gpio_set('E', 9, DIGITAL_OUT);
    adc_init();
    timer2_init();
    dma_adc_init();
    adc_start_audio_stream();
  


    while (1){
        time_cycles = current_cycles - prev_cycles;
    } 
}
