#include "system.h"

static volatile uint32_t g_micros;
volatile struct fault_info g_fault;

__attribute__((used, noreturn))
void hardfault_c(uint32_t *frame)
{
    g_fault.cfsr = SCB->CFSR;
    g_fault.hfsr = SCB->HFSR;
    g_fault.bfar = SCB->BFAR;
    g_fault.pc   = frame[6];
    g_fault.lr   = frame[5];

    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) __BKPT(0);
    NVIC_SystemReset();
    for (;;) { }
}


/* Timing */
uint32_t micros(void){
    return g_micros;
}

uint32_t millis(void){
    return g_micros / 1000U;
}


void clock_init_64mhz_hsi(void)
{
    /* 1. Flash wait states UP before the clock goes up.
          2 WS covers anything above 48 MHz. */
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;   /* 010 */

    /* 2. HSI is already on out of reset, but don't assume — you may be
          re-entering this after a failed HSE attempt. */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) { }

    /* 3. Make sure we're not running off the PLL while we reconfigure it. */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) { }
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY) { }

    /* 4. Bus prescalers BEFORE the switch, or APB1 briefly sees 64 MHz. */
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2))
              | RCC_CFGR_HPRE_DIV1      /* AHB  = 64 MHz */
              | RCC_CFGR_PPRE1_DIV2     /* APB1 = 32 MHz */
              | RCC_CFGR_PPRE2_DIV1;    /* APB2 = 64 MHz */

    /* 5. PLL: HSI/2 = 4 MHz, ×16 = 64 MHz. */
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL))
              | RCC_CFGR_PLLSRC_HSI_DIV2
              | RCC_CFGR_PLLMUL16;      /* field 1110 */

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) { }

    /* 6. Switch. */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { }
}


void generic_fault_handler(){
    for(;;){ }
}