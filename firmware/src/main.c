#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/dac.h"
#include "drivers/dma.h"
#include "drivers/gpio.h"
#include "drivers/tim2.h"
#include "drivers/delay.h"
#include "drivers/spi.h"
#include "drivers/ili9341.h"
#include "misc/spectrum.h"
#include "dsp/fft.h"
#include "misc/textfont.h"
#include "misc/bands.h"
#include "misc/button.h"
#include "system/system.h"

int main(void)
{
    delay_init();

    // Display init

    SPI_Init();
    ILI9341_Init();
    ILI9341_SetRotation(0);           /* landscape: 320×240 */
    Spectrum_DrawAxes();
    bands_init();


    // Peripherics Init
    
    timer2_init();
    adc_init();
    gpio_set('A', 1, ANALOG);
    dac_init();
    dma_adc_init();
    dma_dac_init();
    button_init();

    float energy[24];
    uint8_t magnitudes[24] = {0};
    uint8_t tmpx = 50;

    button_wait_press();

    for(uint8_t i = 3; i > 0; --i){
        
        draw_string(50,50, "Starting AUDIO Stream in");
        draw_char(tmpx,60, i + 48);
        delay_ms(1000);
        tmpx += 10;
    }


    adc_start_audio_stream();
    
    // Super Loop

    while(1){

        if(hop_ready){
            hop_ready = 0;

            frame_prepare(fft_sample_ring, start_index, fft_frame_re, fft_frame_im);
            fft_radix2(fft_frame_re, fft_frame_im, FFT_FRAME_SIZE);
            bands_compute(fft_frame_re, fft_frame_im, energy);
            bands_to_bars(energy, magnitudes);
            Spectrum_Update(magnitudes);
            delay_ms(10);
        }

    }

}
