# STM32 两轮自平衡车｜姿态检测、自动平衡与蓝牙遥控

基于 STM32F103C8T6、MPU6050 DMP、TB6612 电机驱动和增量式编码器的两轮自平衡车固件。工程使用裸机超级循环，不依赖 RTOS；控制算法、硬件驱动和应用调度按职责拆分，便于实车调试和后续扩展。

## 它怎么保持平衡

MPU6050 通过 DMP 输出车体倾角和角速度，编码器提供两侧车轮的速度反馈。速度 PI 根据“目标速度与实际速度的差”产生目标倾角，角度 PD 再根据倾角与角速度计算电机输出；转向指令混入左右轮输出，由 TB6612 驱动车轮纠正倾倒趋势。

```text
MPU6050 姿态 + 编码器速度
           ↓
速度 PI → 目标倾角 → 角度 PD
           ↓
转向混控 → 左右电机 PWM → 车轮运动 → 传感器反馈
```

适合阅读两轮自平衡车的级联控制实现，或在对应硬件上继续调试。**本仓库包含固件源码；控制方向、参数和实车平衡效果仍需按硬件检查清单验证。**

## 从哪里开始

- 想编译：用 Keil MDK 打开 [Project.uvprojx](Project.uvprojx)。
- 想理解算法：先读 [Control/balance_controller.c](Control/balance_controller.c) 和 [控制算法说明](docs/04_control_algorithm.md)。
- 想接线和上电：先看 [硬件与引脚](docs/02_hardware_and_pins.md) 与 [上电检查清单](docs/06_bringup_checklist.md)。

## 功能

- MPU6050 DMP FIFO 提供姿态与角速度，控制频率目标为 100 Hz。
- 上电完成 IMU 初始化后，车体需保持稳定约 0.5 秒，随后自动进入平衡控制。
- 蓝牙串口使用 USART3；未收到指令时保持零速度、零转向，连接蓝牙不会影响自动平衡。
- 速度 PI、角度 PD 与转向混控均使用明确单位：degree、deg/s、encoder counts/s。
- IMU 超时、FIFO 读取失败、控制周期异常或倾倒时撤销电机授权。
- OLED 分页显示姿态、速度、电池原始值和诊断计数。

## 硬件

| 模块 | 引脚/外设 |
| --- | --- |
| 左电机编码器 / PWM / 方向 | PA0/PA1, PA8, PB14/PB15; TIM2, TIM1_CH1 |
| 右电机编码器 / PWM / 方向 | PB6/PB7, PA11, PB13/PB12; TIM4, TIM1_CH4 |
| MPU6050 | PB3/PB4/PB5 软件 I2C，PB5 EXTI5 数据就绪 |
| OLED / 蓝牙 | PB8/PB9 软件 I2C，PB10/PB11 USART3 |
| 电池检测 | PA4 ADC |

完整连线与方向确认见 [硬件与引脚](docs/02_hardware_and_pins.md)，原理图数据见 `schematic_doc.json`。

## 构建

1. 使用 Keil MDK 打开 `Project.uvprojx`。
2. 选择 `Target 1` 并执行 Build。
3. 固件默认输出为 `Objects/Project.hex`。

工程使用 ARMCC 5 兼容的 C99 配置。主机纯算法测试可在仓库根目录用 GCC 执行：

```powershell
gcc -std=c99 -Wall -Wextra -IConfig -IApp -IControl tests/host/test_control.c App/app_state.c App/remote_control.c Control/pid.c Control/filters.c Control/motor_mixer.c Control/balance_controller.c -lm -o tests/host/test_control.exe
./tests/host/test_control.exe
```

## 蓝牙指令

蓝牙通过 USART3 接收单字符指令：`Z` 停止、`E/A` 前进/后退、`B/H` 与 `C/G` 转向、`X/Y` 调整速度等级。通信中断只负责接收字节；控制计算仍在主循环中完成。

## 目录

```text
App/       应用调度、状态机和蓝牙协议
BSP/       STM32 外设与板级封装
Config/    控制、应用和硬件方向配置
Control/   可主机测试的 PI、滤波、混控和级联控制
Core/      启动入口和中断入口
Drivers/   MPU/DMP、TB6612 和 SSD1306 驱动
Hardware/MPU6050/eMPL/  InvenSense DMP 第三方源码
docs/      架构、硬件、控制和上电文档
tests/host/ 主机控制算法测试
```

## 安全说明

控制参数、IMU 安装方向、电机极性和编码器符号尚需实车验证。首次上电必须架空车轮，并按[上电检查清单](docs/06_bringup_checklist.md)逐项确认；不要在未完成方向和低 PWM 验证前让小车落地运行。

## 文档

- [架构](docs/01_architecture.md)
- [硬件与引脚](docs/02_hardware_and_pins.md)
- [运行时与中断](docs/03_runtime_and_interrupts.md)
- [控制算法](docs/04_control_algorithm.md)
- [调参指南](docs/05_tuning_guide.md)
- [上电检查清单](docs/06_bringup_checklist.md)
- [代码阅读顺序](docs/07_code_reading_order.md)
- [第三方代码说明](docs/third_party.md)
