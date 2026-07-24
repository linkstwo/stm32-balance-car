# 控制算法

两轮平衡车是倒立摆：车体倾斜会产生重力矩，电机必须在车轮下方产生相反加速度。控制器使用同一 DMP FIFO 包的 Pitch（degree）和 gyro Y/Z（deg/s），不再把寄存器陀螺仪与四元数跨采样拼接，也不再使用 `gyro_z + 13`。

1. 遥控目标先经过速度斜坡，单位为编码器 counts/s；未掌握 PPR、减速比和轮径前不伪造 m/s。
2. 速度低通后进入 20 Hz PI：误差为 `target - measured`，积分乘 `dt`，带积分与输出限幅、条件积分抗饱和；输出是期望俯仰角偏置（degree）。
3. 100 Hz 角度 PD 以 `pitch - target_pitch` 和 gyro Y deg/s 产生基础电机力矩。
4. 转向使用目标 yaw rate 与 gyro Z deg/s 的误差。
5. 混控为 `left=balance-turn`、`right=balance+turn`；先限制转向量，尽量保留平衡基础力矩。

`Legacy/legacy_controller.c` 保存旧参数和结构作对照，但它依赖旧的 raw gyro 单位，默认不会运行。SafeBringup 与 CascadeV1 参数在 `balance_controller.c`，全部为 `UNVERIFIED_ON_HARDWARE`；不要直接把旧 `Vertical_Kd=-2.2` 套到 deg/s 新算法。旧 `Encoder_sum += Movement` 不是标准积分，已由明确的 `error * dt` 替代。
