# stm32-balance-car

STM32 balance car project.
This version keeps only Bluetooth remote control and does not include OpenMV code.

## Notes

- Project file: `Project.uvprojx`
- Toolchain: Keil MDK
- Entry file: `User/main.c`
- The remote side must keep sending valid control commands with a period below 300 ms, or the car will stop automatically.
- After MPU fault latch, motor output will not recover automatically. Power-cycle recovery is required.