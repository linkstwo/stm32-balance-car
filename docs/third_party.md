# 第三方代码说明

- `Libraries/`：ST STM32F10x Standard Peripheral Library，版本由原工程保留。默认 Keil 构建仅编译 GPIO、RCC、TIM、USART、EXTI、ADC 和 `misc`。
- `Startup/`：CMSIS 和 STM32F103 启动代码，保留版权头，不做项目风格重写。
- `Hardware/MPU6050/eMPL/`：InvenSense DMP 源码和固件，保留版权头和原有主体实现。

eMPL 原文件带有历史 STM32 移植宏，默认路径不再调用其中追加的 `mpu_dmp_init()` 和 `mpu_dmp_get_data()` 业务包装。新的项目适配位于 `Drivers/IMU/mpu_dmp_adapter.c`，I2C/延时兼容接口集中在 `Drivers/IMU/inv_mpu_port.c`。保留旧符号仅为兼容未改动的厂商源，不向业务层导出。

第三方文件没有按本项目命名风格整体重写，原因是保持供应商代码可审计、降低 DMP 固件升级和差异比较风险。
