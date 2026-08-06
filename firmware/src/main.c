
#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "drivers/spi.h"
#include "misc/MAX7219.h"

/* ---- main --------------------------------------------------------- */
int main(void)
{
    render(184632U);
}
