#ifndef SYSTEM_H
#define SYSTEM_H
#include <stdint.h>
#include "stm32f303xc.h"

#define SYS_CLK 64000000U

struct fault_info {
    uint32_t cfsr;   /* why it faulted   */
    uint32_t hfsr;   /* forced/escalated */
    uint32_t bfar;   /* offending address */
    uint32_t pc;     /* faulting instruction */
    uint32_t lr;     /* caller */
};


void hardfault_c(uint32_t *frame)  __attribute__((used, noreturn));

void clock_init_64mhz_hsi(void);
void clock_init_72mhz_hse(void);

void Init_Handlers(void);
void SysTick_Handler(void);

inline uint32_t micros(void);
inline uint32_t millis(void);
#endif