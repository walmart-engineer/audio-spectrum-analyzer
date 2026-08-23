#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "drivers/spi.h"
#include "drivers/ili9341.h"
#include "misc/spectrum.h"
#include "dsp/fft.h"
#include "misc/textfont.h"

int main(void)
{
    delay_init();
    SPI_Init();
    SPI_DMA_Init();

    ILI9341_Init();
    ILI9341_SetRotation(0);           /* landscape: 320×240 */

    /* Draw the static purple axes and labels (once) */
    Spectrum_DrawAxes();

    uint8_t magnitudes[24] = {0};
    
    while(1){

        if(hop_ready){
            hop_ready = 0;

            frame_prepare(fft_sample_ring, start_index, fft_frame_re, fft_frame_im);
            fft_radix2(fft_frame_re, fft_frame_im, FFT_FRAME_SIZE);
            
        }

    }

}
