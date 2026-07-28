# 代码阅读顺序

1. `Core/main.c`：理解超级循环的优先级和无阻塞原则。
2. `App/app.c`：理解初始化顺序、数据如何进入控制器和状态机如何授权电机。
3. `App/app_state.c`：理解 ARM、遥控丢失、倾倒和 IMU 故障转移。
4. `Config/board_config.h`：理解硬件符号和哪些值必须实车确认。
5. `BSP/timebase.c`、`BSP/uart3.c`、`BSP/encoder.c`：理解 STM32 定时器、串口中断和编码器采样。
6. `Drivers/IMU/mpu_dmp_adapter.c`：理解 DMP FIFO 如何形成带单位的 `ImuSample`。
7. `Control/balance_controller.c`、`pid.c`、`motor_mixer.c`：理解级联环路、抗饱和和混控。
8. `Drivers/Motor/tb6612.c`：理解电机方向、Coast、Brake 和统一 PWM 限幅。
9. `Drivers/Display/ssd1306.c`：理解低优先级诊断显示。

看完每步后应能回答：输入从哪里来、单位是什么、在哪个上下文运行、失败会进入什么状态。
