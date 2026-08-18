#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "drivers/spi.h"
#include "drivers/ili9341.h"
#include "spectrum.h"

int main(void)
{
    delay_init();
    SPI_Init();
    SPI_DMA_Init();

    ILI9341_Init();
    ILI9341_SetRotation(0);           /* landscape: 320×240 */

    /* Draw the static purple axes and labels (once) */
    Spectrum_DrawAxes();

    /* --- demo: animated sweep so you can see the bars move ---------- */
    uint8_t magnitudes[24] = {0};
    uint8_t phase = 0;

    for (;;) {
        /* Generate a simple sine-ish sweep pattern for testing.
           Replace this with real ADC/FFT data in production. */
        for (uint8_t i = 0; i < 24; ++i) {
            int16_t v = 255 - (int16_t)((uint16_t)(i * 11 + phase) % 256);
            if (v < 0) v = -v;
            magnitudes[i] = (uint8_t)v;
        }

        Spectrum_Update(magnitudes);
        phase += 3;

        delay_ms(1);      /* ~33 FPS target */
    }
}
