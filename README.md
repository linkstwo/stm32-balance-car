# stm32-balance-car

STM32 balance car project.
This version keeps only Bluetooth remote control and does not include OpenMV code.

## Notes

- Project file: `Project.uvprojx`
- Toolchain: Keil MDK
- Entry file: `User/main.c`
- The remote side must keep sending valid control commands with a period below 300 ms, or the car will stop automatically.
- After MPU fault latch, motor output will not recover automatically. Power-cycle recovery is required.

## Board pin mapping

The firmware targets the STM32F103C8T6 board schematic stored in `schematic_doc.json`.

| Function | STM32 pin | Peripheral |
| --- | --- | --- |
| M1 encoder | PA0 / PA1 | TIM2 CH1 / CH2 |
| M1 PWM | PA8 | TIM1 CH1 |
| M1 direction | PB14 / PB15 | TB6612 AIN1 / AIN2 |
| M2 encoder | PB6 / PB7 | TIM4 CH1 / CH2 |
| M2 PWM | PA11 | TIM1 CH4 |
| M2 direction | PB13 / PB12 | TB6612 BIN1 / BIN2 |
| MPU6050 I2C | PB3 / PB4 | software I2C SDA / SCL |
| MPU6050 interrupt | PB5 | EXTI5, falling edge |
| OLED | PB8 / PB9 | SCL / SDA |
| Bluetooth | PB10 / PB11 | USART3 TX / RX |
| Reserved ultrasonic interface | PA2 / PA3 | SR04 Echo / Trig |
| Reserved battery input | PA4 | battery divider ADC |

`Board_PinMux_Init()` disables JTAG and preserves SWD, which releases PB3/PB4 for the MPU6050. The default startup path does not call `USART2_Init()` or `Button_Init()`, so PA2/PA3 remain reserved for the ultrasonic interface and PA4 remains available for battery ADC sampling. USART3 is reserved for Bluetooth control, not debug logging.

## Hardware validation required

This repository is statically checked only. Before driving the car, test it with the wheels raised: confirm M1 and M2 affect only their respective TB6612 channels, verify both encoder directions, and confirm the MPU6050 pitch compensation direction. The left/right physical wheel positions, motor polarity, encoder polarity, and MPU6050 installation orientation must be verified on the assembled car.
