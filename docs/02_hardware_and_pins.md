# 硬件和引脚

`schematic_doc.json` 是硬件真相。启动时关闭 JTAG、保留 PA13/PA14 SWD，释放 PB3/PB4。

| 网络 | 引脚 | GPIO/外设模式 | 说明 |
| --- | --- | --- | --- |
| M1A/M1B | PA0/PA1 | TIM2 编码器输入 | M1，符号待悬空确认 |
| PWMA | PA8 | TIM1_CH1 AF 推挽 | 10 kHz 初始 PWM |
| AIN1/AIN2 | PB14/PB15 | 推挽输出 | TB6612 A |
| M2A/M2B | PB6/PB7 | TIM4 编码器输入 | M2，符号待悬空确认 |
| PWMB | PA11 | TIM1_CH4 AF 推挽 | 10 kHz 初始 PWM |
| BIN1/BIN2 | PB13/PB12 | 推挽输出 | TB6612 B |
| 6050_SDA/SCL | PB3/PB4 | 开漏软件 I2C | 外部上拉和总线恢复 |
| 6050_INT | PB5 | 上拉输入 / EXTI5 下降沿 | ISR 只记事件 |
| OLED_SCL/SDA | PB8/PB9 | 开漏软件 I2C | 分页 burst 写入 |
| USART3_TX/RX | PB10/PB11 | USART3 | 仅蓝牙控制协议 |
| SR04_Echo/Trig | PA2/PA3 | 保留 | 默认不初始化 USART2 |
| ADC | PA4 | ADC1_CH4 模拟输入 | 默认仅原始采样 |

原理图中电池链路标为 `VBAT_IN`、R1=20K、R2=20K、R3=10K，理论 `Vadc=Vbat/5`。仍需万用表确认实际阻值、VDDA 和 ADC 基准后才能启用电压换算与低压保护。

需要实车确认：M1/M2 对应左右轮、M+/M- 极性、编码器相位符号、MPU6050 安装方向、平衡零点和 OLED 朝向。
