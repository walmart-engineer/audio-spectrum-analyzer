#include "stm32f303xc.h"
#include "drivers/adc.h"
#include "drivers/delay.h"
#include "system/system.h"  
#include "drivers/gpio.h"

#define MIC_PIN 1u
#define MIC_PORT 'A'

static uint32_t vdda; // VDDA in mV

/* ---- main --------------------------------------------------------- */

int main(void){
    
}
