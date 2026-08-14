#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "system/system.h"  
#include "drivers/gpio.h"
#include "misc/MAX7219.h"
#include "drivers/dma.h"
#include "drivers/tim2.h"

int main(void)
{
  uint8_t volatile button_pressed = 0u;
  gpio_set('A',0, DIGITAL_IN);
  gpio_set('A',1,ANALOG);
  gpio_set('E', 9, DIGITAL_OUT);
  adc_init(); //config ADC1
  timer2_init(); //config TIM2
  dac_init(); //config DAC, activ cererile DMA si umple dac_buffer cu 2048
  dma_adc_init(); //config DMA1 pe canal 1
  dma_dac_init(); //config DMA1 pe canal 3
  adc_start_audio_stream(); //activeaza TIM2, ADC1 si DAC
  
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
