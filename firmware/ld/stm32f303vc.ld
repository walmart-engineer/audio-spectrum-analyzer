/*
 * stm32f303vc.ld — hand-written linker script for STM32F303VCT6
 *
 * This file is the map of the chip's memory. The linker uses it to assign
 * every function and variable a real address. The three regions below come
 * straight from the RM0316 reference manual / datasheet memory map.
 */

MEMORY
{
    FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 256K   /* code + const     */
    RAM    (rwx) : ORIGIN = 0x20000000, LENGTH = 40K    /* data, bss, DMA   */
    CCMRAM (rwx) : ORIGIN = 0x10000000, LENGTH = 8K     /* CPU-only, no DMA */
}

/* Stack grows DOWN from the very end of RAM. This symbol becomes entry 0
 * of the vector table — the hardware loads it into SP at reset.          */
_estack = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(Reset_Handler)

SECTIONS
{
    /* The vector table MUST be the first bytes of flash: the Cortex-M4
     * reads SP and PC from offsets 0x0 and 0x4 at reset. KEEP() stops
     * the linker from garbage-collecting it (nothing "calls" it).       */
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    /* Code and read-only data (string literals, const tables — later,
     * the FFT twiddle/window/bit-reversal tables land here for free).   */
    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* Initialized globals: LIVE in RAM at runtime, but their initial
     * values must be STORED in flash (RAM is garbage at power-up).
     * "> RAM AT > FLASH" expresses exactly that dual placement; the
     * Reset_Handler copy loop bridges the two using these symbols.      */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* Zero-initialized globals — occupy RAM, cost nothing in flash.     */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* CPU-only fast memory. Empty today, but the section exists from
     * day one so that placing the Q15 FFT working buffer later is one
     * attribute, not a linker-script rewrite:
     *   __attribute__((section(".ccmram"))) int16_t fft_work[4096];
     * NOTE: nothing initializes this region — treat contents as garbage
     * until written (or add a copy/zero loop to Reset_Handler when the
     * first .ccmram variable appears).                                  */
    .ccmram (NOLOAD) :
    {
        . = ALIGN(4);
        *(.ccmram*)
    } > CCMRAM

    /* Debug info stays in the ELF for gdb; objcopy strips it from .bin */
}
