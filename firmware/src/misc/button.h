#include "stm32f303xc.h"

#define BTN_PORT   GPIOA
#define BTN_PIN    0u

void button_init(void);
void button_wait_press(void);