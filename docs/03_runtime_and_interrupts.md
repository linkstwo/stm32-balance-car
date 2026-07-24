# 运行时和中断

TIM3 在 1 MHz 计数、1 ms 更新中断下维护单调 `uint32_t` 毫秒时钟。所有超时采用 `(uint32_t)(now - start) >= timeout`，可跨回绕。初始化阶段的短延时使用 TIM3 计数；运行期主循环没有 `delay_ms`。

中断优先级：EXTI5 为 0（最高业务优先级）、USART3 为 1、TIM3 为 2。配置只在 `App_ConfigureInterruptPriorities()` 一处完成。

- EXTI5：清标志、记录时间、饱和增加 IMU pending 计数。
- USART3：读取一个字节，写入 64 字节环形缓冲，记录接收时间。
- TIM3：仅递增毫秒计数。

ISR 中禁止软件 I2C、DMP FIFO、OLED、浮点控制、`printf` 和阻塞延时。主循环优先消费 IMU 事件，目标控制频率为 100 Hz；速度 PI 为 20 Hz；OLED 每 200 ms 更新并每次仅发送一页。`AppContext` 记录最大控制周期和 overrun 次数；控制间隔不在 5 到 20 ms 内时撤销本周期输出。
