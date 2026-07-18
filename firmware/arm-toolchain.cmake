# arm-toolchain.cmake — tells CMake to cross-compile for Cortex-M4
# Used as: cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake

set(CMAKE_SYSTEM_NAME       Generic)   # bare metal: no OS
set(CMAKE_SYSTEM_PROCESSOR  arm)

set(CMAKE_C_COMPILER    arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_OBJCOPY       arm-none-eabi-objcopy)
set(CMAKE_SIZE          arm-none-eabi-size)

# CMake's compiler sanity test tries to build & RUN a program — impossible
# when the output is ARM code on an x86 host. Test compile-only instead.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# The ONE place the CPU/FPU ABI is defined. Every file — C and assembly,
# compile and link — inherits these. Mixing float ABIs across objects is
# a silent-corruption bug; centralizing here makes it impossible.
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT   "${MCU_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCU_FLAGS} -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs")

# -ffunction-sections/-fdata-sections + --gc-sections: every function gets
# its own section and the linker discards unreferenced ones — dead code
# costs zero flash.
# nano.specs: newlib-nano, the small embedded libc build.
# nosys.specs: stub syscalls (bare metal has no OS to call).
