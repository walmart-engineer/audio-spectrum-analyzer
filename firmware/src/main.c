#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "system/system.h"  
#include "drivers/gpio.h"
#include "misc/MAX7219.h"
#include "drivers/dma.h"
#include "drivers/tim2.h"

extern volatile uint8_t gata;
int main(void)
{
  uint8_t volatile button_pressed;
  gpio_set('A',0, DIGITAL_IN);
  gpio_set('A',1,ANALOG);
  gpio_set('E', 9, DIGITAL_OUT);
  adc_init();
  timer2_init();
  dma_adc_init();
  adc_start_audio_stream();
  
  while (1){
  if(gata)
    {
      gpio_write('E', 9, HIGH);
      delay_ms(100);

      while(!button_pressed) { 
        button_pressed = gpio_digital_read('A', 0);
      }
    }

    }
   
}
