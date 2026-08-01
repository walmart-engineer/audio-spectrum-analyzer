# blinky — STM32F3 Discovery, register-level, no HAL

Week-1 gate project: hand-written startup, hand-written linker script,
three-register blinky on PE8 (LD4, blue). Verified to build with
arm-none-eabi-gcc 13.2 (560 bytes of flash).

## Layout
    src/main.c            the blinky (RCC clock enable, MODER, BSRR)
    src/startup.c         vector table + Reset_Handler (.data copy, .bss zero)
    ld/stm32f303vc.ld     memory map: FLASH 256K / RAM 40K / CCMRAM 8K
    cmsis/                vendored CMSIS headers (ST + ARM official repos)
    tools/STM32F303.svd   peripheral register descriptions for Cortex-Debug
    arm-toolchain.cmake   cross-compiler + Cortex-M4 hard-float ABI flags
    .vscode/              F5 debug config, build tasks, extension recs

## Build & flash (command line)
    cmake -B build -G Ninja "-DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake"
    cmake --preset default
    ninja -C build
    ninja -C build flash        # board on the USB ST-LINK connector

## Debug (VS Code)
Install recommended extensions when prompted, then press F5.
Toolchain versions used: record yours here for reproducibility.
