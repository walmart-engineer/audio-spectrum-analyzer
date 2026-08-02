/* ============================================================
   STM32F303VCT6 — 256 KB flash, 40 KB SRAM, 8 KB CCM
   ============================================================ */

ENTRY(Reset_Handler)        /* first function to run; also marks it as
                                "used" so --gc-sections won't delete it */

_estack = 0x2000A000;       /* top of RAM — initial stack pointer */

_Min_Heap_Size  = 0x000;    /* no malloc on this project */
_Min_Stack_Size = 0x800;    /* 2 KB reserved for the stack */

MEMORY
{
  FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
  RAM    (xrw) : ORIGIN = 0x20000000, LENGTH = 40K
  CCMRAM (xrw) : ORIGIN = 0x10000000, LENGTH = 8K
}

SECTIONS
{
  /* ---- Vector table: MUST be first, at 0x08000000 ---------
     KEEP() is essential: nothing in C ever "calls" the vector
     table, so --gc-sections would otherwise discard it and the
     chip would boot into nothing.                            */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* ---- Code ---------------------------------------------- */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)          /* .text.main, .text.fft_process, … one per
                          function when -ffunction-sections is on */
    *(.glue_7)         /* ARM/Thumb interworking stubs */
    *(.glue_7t)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  /* ---- Read-only data: const tables live and stay here ---- */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  /* ---- C++ / unwinding tables (harmless, keep them) ------- */
  .ARM.extab : { *(.ARM.extab*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  /* ---- Initialized variables ------------------------------
     The one section with two addresses. Runs from RAM,
     initial values stored in flash.                */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  /* ---- Zero-initialized variables -------------------------
     Costs RAM, costs ZERO flash. All DMA buffers live here
     because DMA can only reach the 0x20… region.            */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* ---- CCM: CPU-only fast RAM ----------------------------
     NOLOAD = reserve the space, store nothing in the image.
     NEVER put a DMA buffer here — DMA cannot see 0x10…      */
  .ccmram (NOLOAD) :
  {
    . = ALIGN(4);
    _sccmram = .;
    *(.ccmram)
    *(.ccmram*)
    . = ALIGN(4);
    _eccmram = .;
  } >CCMRAM

  /* ---- Reserve room so overflow is a BUILD error ---------- */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* ---- Throw away compiler/library metadata we don't need - */
  /DISCARD/ :
  {
    libc.a(*)
    libm.a(*)
    libgcc.a(*)
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}

ASSERT(_ebss + _Min_Stack_Size <= ORIGIN(RAM) + LENGTH(RAM),
       "ERROR: RAM overflowed — no room left for the stack")