# 迁移说明

基线提交：`c6db8b6ee87180a7454f39503b69becb8d28d75f`。本地备份标签：`backup/pre-refactor-20260724-1154`。

新目录树：

```text
Core/       启动和集中 IRQ
Config/     板级、应用和控制参数
BSP/        TIM3、GPIO、PWM、编码器、UART、ADC、软件 I2C
Drivers/    TB6612、MPU/DMP 适配、SSD1306
Control/    可主机测试的 PI、滤波、混控和级联控制
App/        调度、状态机、遥控协议
Legacy/     旧算法对照，不是默认路径
docs/       学习与上电文档
tests/host/ GCC 纯算法测试
```

| 旧模块/变量 | 新位置 | 行为 |
| --- | --- | --- |
| `System/sys.h` | 各模块精确头文件 | 不再作为业务依赖入口 |
| `User/main.c` | `Core/main.c` + `App/app.c` | 主循环事件化，无 50 ms 阻塞 |
| `control.c` ISR | `Core/stm32f10x_it.c` + `Control/` | ISR 只发事件，控制在主循环 |
| `Flag_*` / `Speed_Times` | `RemoteControl` | 无业务全局变量，协议兼容 |
| `Read_Spead` | `Encoder_ReadDeltaCounts` | 明确是周期计数增量 |
| `SETPWM` | `MotorDriver_Set/Coast/Brake` | 零输出方向明确，统一限幅 |
| `MPU_Init + mpu_dmp_init` | `Imu_Init` | 单一项目级入口 |
| `OLED_*` 旧驱动 | `ssd1306.c` | framebuffer、burst、分页刷新 |
| USART2/Button | 保留旧源但移出默认构建 | PA2/PA3、PA4 不再冲突 |

旧 `Hardware`、`System` 与 `User` 源文件保留用于对照，不参与默认构建；`Legacy/legacy_controller.c` 是可选参考实现。蓝牙单字符协议保持：`Z/E/A/C/G/B/H/X/Y`。`X/Y` 在内部转换为直观的 speed level，但保留旧手机端的实际快慢效果。

当前本地构建和主机测试通过；实际电机、编码器、IMU 和参数验证仍未完成。第一轮上电必须执行 [bringup checklist](06_bringup_checklist.md)。

## 本次验证报告

- Keil MDK / ARMCC 5：`0 Error(s), 0 Warning(s)`，生成 `Objects/Project.hex`。
- 映像：Code=24380、RO=5684、RW=1036、ZI=2284 bytes；估算 Flash=31100 bytes、RAM=3320 bytes。
- 主机测试：`host control tests passed`，覆盖 PI 零误差与抗饱和、遥控超时斜坡、IMU 故障撤销授权、倾倒再授权迟滞、混控限幅和 `uint32_t` 回绕。
- 未完成实车验证：M1/M2 左右关系、电机极性、编码器符号、MPU 安装方向、零点角、全部控制参数、OLED 电气上拉、ADC 参考和电池分压实测。

推荐下一步：先执行悬空上电清单，再确认各方向和 Pitch 符号；之后只开低 PWM 角度 PD，最后依次调速度 PI、转向和 PWM 限幅。
