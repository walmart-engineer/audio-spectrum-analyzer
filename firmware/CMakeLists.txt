cmake_minimum_required(VERSION 3.20)
project(blinky C ASM)

set(CMAKE_C_STANDARD 11)

add_executable(blinky.elf
    src/main.c
    src/startup.c
    src/drivers/tim2.c
    src/drivers/delay.c
    src/drivers/adc.c
    src/drivers/spi.c
    src/drivers/ili9341.c
    src/misc/spectrum.c
    src/misc/MAX7219.c
    src/drivers/spi.c
    src/drivers/dma.c
    src/drivers/dac.c
    src/dsp/twiddle.c
    src/dsp/window.c
    src/dsp/fft.c
    src/dsp/decimate.c
    src/system/system.c
)

target_include_directories(blinky.elf PRIVATE cmsis src)

# The device header (stm32f3xx.h) dispatches on this define to pull in
# the right register file (stm32f303xc.h) for our exact chip.
target_compile_definitions(blinky.elf PRIVATE STM32F303xC)

target_compile_options(blinky.elf PRIVATE
    -Wall -Wextra
    -Og -g3            # optimize-for-debug: fast enough, still steppable
)

# =========================== LINKER======================================
# Hand the linker OUR linker script, and re-link if the script changes.
target_link_options(blinky.elf PRIVATE
    -T${CMAKE_SOURCE_DIR}/ld/stm32f303_2.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/blinky.map   # full address map, worth reading once
)
set_target_properties(blinky.elf PROPERTIES
    LINK_DEPENDS ${CMAKE_SOURCE_DIR}/ld/stm32f303_2.ld)
# ========================================================================
# Post-build: raw binary image + the flash/RAM fuel gauge.
add_custom_command(TARGET blinky.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:blinky.elf> ${CMAKE_BINARY_DIR}/blinky.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:blinky.elf>
    COMMENT "Generating blinky.bin and size report")

# `ninja flash` — the whole pipeline in one command:
# rebuild stale files -> relink -> program flash -> verify -> reset -> run
add_custom_target(flash
    COMMAND openocd -f board/stm32f3discovery.cfg
            -c "program {$<TARGET_FILE:blinky.elf>} verify reset exit"
    DEPENDS blinky.elf
    COMMENT "Flashing via OpenOCD/ST-Link")
