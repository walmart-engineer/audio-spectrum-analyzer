/*
 * startup.c — hand-written startup for STM32F303VCT6 (no HAL, no vendor .s file)
 *
 * This file is what runs BEFORE main(). It owns two things:
 *   1. The vector table: the array of addresses the Cortex-M4 hardware reads
 *      at reset and on every exception/interrupt. It MUST be the very first
 *      thing in flash (the linker script pins the ".isr_vector" section there).
 *   2. Reset_Handler: the true entry point. It builds the C runtime environment
 *      (copies .data from flash to RAM, zeroes .bss) and then calls main().
 */

#include <stdint.h>
#include "stm32f3xx.h"
#include "system/system.h"

/* Symbols defined by the linker script (ld/stm32f303vc.ld) */
extern uint32_t _estack;   /* top of stack (end of RAM)                    */
extern uint32_t _sidata;   /* start of .data initial values, in FLASH      */
extern uint32_t _sdata;    /* start of .data destination, in RAM           */
extern uint32_t _edata;    /* end of .data in RAM                          */
extern uint32_t _sbss;     /* start of .bss in RAM                         */
extern uint32_t _ebss;     /* end of .bss in RAM                           */



int main(void);

/* ------------------------------------------------------------------ */
/* Default handler: any unexpected exception/interrupt lands here.    */
/* An infinite loop is DELIBERATE: attach the debugger, and the call  */
/* stack tells you exactly which fault fired. Silent resets hide bugs.*/
/* ------------------------------------------------------------------ */
void Default_Handler(void)
{
    for (;;) { }
}

/* Core exception handlers — weak aliases so real implementations can
 * replace them later (e.g. a proper HardFault dumper in week 2+)     */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((naked));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));


/* Peripheral interrupts we'll implement in later milestones get added
 * here the same way, at their correct vector positions, e.g.:
 *   void DMA1_Channel1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
 */

void Reset_Handler(void);

/* ------------------------------------------------------------------ */
/* The vector table.                                                  */
/* Entry 0 is not code — it is the initial stack pointer value the    */
/* hardware loads into SP before executing a single instruction.      */
/* Entry 1 is the reset vector: the address of Reset_Handler.         */
/* The STM32F303 has 82 device IRQs after the 16 core entries; for    */
/* blinky we only populate the core ones and let GCC zero-fill the    */
/* rest (a 0 vector would fault if ever taken — acceptable until we   */
/* actually enable those interrupts, at which point we add them).     */
/* ------------------------------------------------------------------ */
typedef void (*vector_t)(void);

__attribute__((section(".isr_vector"), used))
static const vector_t vector_table[16 + 82] = {
    (vector_t)&_estack,     /*  0: initial stack pointer               */
    &Reset_Handler,          /*  1: reset                               */
    &NMI_Handler,            /*  2: non-maskable interrupt              */
    &HardFault_Handler,      /*  3: all faults, unless escalated ones on*/
    &MemManage_Handler,      /*  4: MPU violation                       */
    &BusFault_Handler,       /*  5: bus error                           */
    &UsageFault_Handler,     /*  6: undefined instr, div-by-0, etc.     */
    0, 0, 0, 0,             /*  7-10: reserved                         */
    &SVC_Handler,            /* 11: supervisor call                     */
    &DebugMon_Handler,       /* 12: debug monitor                       */
    0,                      /* 13: reserved                            */
    &PendSV_Handler,         /* 14: pendable service                    */
    &SysTick_Handler,        /* 15: system tick timer                   */
    /* 16..97: device IRQs — zero-filled for now, added as needed      */
};

/* ------------------------------------------------------------------ */
/* Reset_Handler: build the C world, then enter it.                   */
/* ------------------------------------------------------------------ */
void Reset_Handler(void)
{
    /* 1. Copy initialized globals (.data) from their flash image to RAM.
     *    Before this loop, every non-const global with an initializer
     *    contains garbage — this loop is WHY `int x = 5;` works.       */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* 2. Zero uninitialized globals (.bss) — WHY `int y;` reads as 0. */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    /* 3. No clock setup yet: we run on the 8 MHz HSI the chip boots
     *    with. PLL bring-up to 72 MHz is its own milestone, done
     *    deliberately in C where we can read every RCC bit we set.   */

    // ------ External 8 MHz Crystal Oscilator
    // RCC->CR |= RCC_CR_HSEBYP;
    // RCC->CR |= RCC_CR_HSEON;
    // while (!(RCC->CR & RCC_CR_HSERDY)) { }

    clock_init_64mhz_hsi();

    main();

    /* main() must never return on bare metal — but if it does, stop
     * somewhere the debugger can see rather than executing garbage.  */
    for (;;) { }
}

void HardFault_Handler(void)
{
    __asm volatile (
        "tst   lr, #4        \n"   /* EXC_RETURN bit 2 selects the stack */
        "ite   eq            \n"
        "mrseq r0, msp       \n"
        "mrsne r0, psp       \n"
        "b     hardfault_c   \n"
    );
}



