#include <stdint.h>
#include "soft_i2c.h"
#include "timebase.h"
#include "inv_mpu_port.h"

static SoftI2cBus g_mpu_bus = { GPIOB, GPIO_Pin_4, GPIO_Pin_3, 3U };

void InvMpuPort_Init(void)
{
    SoftI2c_Init(&g_mpu_bus);
}

unsigned char MPU_Write_Len(unsigned char address, unsigned char reg,
                            unsigned char length, const unsigned char *data)
{
    uint8_t buffer[33];
    uint8_t index;

    if (length > 32U)
    {
        return 1U;
    }
    buffer[0] = reg;
    for (index = 0U; index < length; ++index)
    {
        buffer[index + 1U] = data[index];
    }
    return (SoftI2c_Write(&g_mpu_bus, address, buffer, (uint16_t)length + 1U) == SOFT_I2C_OK) ? 0U : 1U;
}

unsigned char MPU_Read_Len(unsigned char address, unsigned char reg,
                           unsigned char length, unsigned char *data)
{
    return (SoftI2c_ReadRegisters(&g_mpu_bus, address, reg, data, length) == SOFT_I2C_OK) ? 0U : 1U;
}

void delay_ms(unsigned long delay_ms_value)
{
    Timebase_DelayMs((uint32_t)delay_ms_value);
}

unsigned long delay_get_ms(void)
{
    return (unsigned long)Timebase_GetMs();
}

void MPU_IIC_Init(void)
{
    InvMpuPort_Init();
}
