#include <stdint.h>

void delay_init_tim2(void);
 
/* Blocks for `ms` milliseconds (clamped to [1, 10000]).
 * Interrupts continue to run normally while blocked.            */
void delay_ms(uint32_t);