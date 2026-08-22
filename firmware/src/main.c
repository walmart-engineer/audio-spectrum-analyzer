#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "drivers/spi.h"
#include "drivers/ili9341.h"
#include "misc/spectrum.h"
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
    
    draw_string(50,50,"white");
    
   
}
