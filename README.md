# STM32 平衡小车

这是一个基于 STM32F103C8T6、STM32F10x 标准外设库和 MPU6050 DMP 的裸机平衡小车工程。重构后的默认路径使用事件驱动超级循环、统一状态机和安全的 SafeBringup 控制 profile；它尚未完成实车验证。

## 快速开始

1. 使用 Keil MDK 打开 `Project.uvprojx`，选择 `Target 1` 并执行 Build。
2. 默认生成 `Objects/Project.hex`。工具链为 ARMCC 5，工程使用 C99。
3. 第一次上电必须拆轮或悬空，按 [docs/06_bringup_checklist.md](docs/06_bringup_checklist.md) 操作。

## 硬件和安全

| 功能 | 引脚 | 外设 |
| --- | --- | --- |
| M1 编码器 / PWM / 方向 | PA0/PA1、PA8、PB14/PB15 | TIM2、TIM1_CH1、TB6612 A |
| M2 编码器 / PWM / 方向 | PB6/PB7、PA11、PB13/PB12 | TIM4、TIM1_CH4、TB6612 B |
| MPU6050 | PB3/PB4/PB5 | 开漏软件 I2C、EXTI5 |
| OLED / 蓝牙 | PB8/PB9、PB10/PB11 | 软件 I2C、USART3 |
| 预留 | PA2/PA3、PA4 | SR04、ADC 电池采样 |

上电默认 `DISARMED`，只有 MPU 自检通过、姿态稳定且收到 `Z` 中立命令后才能进入 `ARMED`。`REMOTE_LOST` 会平滑将目标归零并继续平衡；倾倒、IMU 过期或 IMU 故障会撤销电机授权。默认参数均为 `UNVERIFIED_ON_HARDWARE`。

## 代码入口

从 [Core/main.c](Core/main.c) 开始，再阅读 `App/app.c`、`App/app_state.c`、`Drivers/IMU/mpu_dmp_adapter.c`、`Control/balance_controller.c` 和 `Drivers/Motor/tb6612.c`。完整路线见 [docs/07_code_reading_order.md](docs/07_code_reading_order.md)。

## 文档

- [架构](docs/01_architecture.md)
- [硬件和引脚](docs/02_hardware_and_pins.md)
- [运行时和中断](docs/03_runtime_and_interrupts.md)
- [控制算法](docs/04_control_algorithm.md)
- [调参指南](docs/05_tuning_guide.md)
- [上电检查清单](docs/06_bringup_checklist.md)
- [迁移说明](docs/08_migration_notes.md)
- [第三方说明](docs/third_party.md)

## 当前验证状态

Keil 构建和主机纯算法测试是软件验证，不代表电机、编码器、MPU 安装方向或控制参数已经过实车验证。不得在未完成悬空检查前将车轮落地运行。
