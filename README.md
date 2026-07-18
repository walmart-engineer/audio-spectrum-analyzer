# Real-Time Audio Spectrum Analyzer

STM32F303VCT6 Discovery · register-level firmware (no HAL) · ILI9341 display
Electret mic + line-in → 192 kHz ADC → ÷4 decimation → 4096-pt FFT → 24-band display + pass-through DAC out.

## Repo layout
    firmware/    CMake + arm-none-eabi-gcc project (see firmware/README-firmware.md)
    docs/        hardware & software guides

## Conventions
- Firmware milestones S1–S5 gate on measured validation (see docs).
- Toolchain versions in use are recorded in firmware/README-firmware.md.
