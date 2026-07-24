#include <stdbool.h>
#include <stdint.h>
#include "soft_i2c.h"
#include "timebase.h"

#define SOFT_I2C_CLOCK_WAIT_LOOPS 100U

static void SoftI2c_Delay(const SoftI2cBus *bus)
{
    Timebase_DelayUs(bus->half_period_us);
}

static void SoftI2c_Release(const SoftI2cBus *bus, uint16_t pin)
{
    GPIO_SetBits(bus->port, pin);
}

static void SoftI2c_DriveLow(const SoftI2cBus *bus, uint16_t pin)
{
    GPIO_ResetBits(bus->port, pin);
}

static bool SoftI2c_IsHigh(const SoftI2cBus *bus, uint16_t pin)
{
    return GPIO_ReadInputDataBit(bus->port, pin) != Bit_RESET;
}

static SoftI2cResult SoftI2c_ReleaseClock(const SoftI2cBus *bus)
{
    uint32_t loops;

    SoftI2c_Release(bus, bus->scl_pin);
    for (loops = 0U; loops < SOFT_I2C_CLOCK_WAIT_LOOPS; ++loops)
    {
        if (SoftI2c_IsHigh(bus, bus->scl_pin))
        {
            SoftI2c_Delay(bus);
            return SOFT_I2C_OK;
        }
        SoftI2c_Delay(bus);
    }
    return SOFT_I2C_ERROR_CLOCK_STRETCH;
}

static SoftI2cResult SoftI2c_Start(const SoftI2cBus *bus)
{
    SoftI2c_Release(bus, bus->sda_pin);
    if (SoftI2c_ReleaseClock(bus) != SOFT_I2C_OK)
    {
        return SOFT_I2C_ERROR_CLOCK_STRETCH;
    }
    if (!SoftI2c_IsHigh(bus, bus->sda_pin))
    {
        return SOFT_I2C_ERROR_BUS_BUSY;
    }
    SoftI2c_DriveLow(bus, bus->sda_pin);
    SoftI2c_Delay(bus);
    SoftI2c_DriveLow(bus, bus->scl_pin);
    SoftI2c_Delay(bus);
    return SOFT_I2C_OK;
}

static void SoftI2c_Stop(const SoftI2cBus *bus)
{
    SoftI2c_DriveLow(bus, bus->sda_pin);
    SoftI2c_Delay(bus);
    SoftI2c_ReleaseClock(bus);
    SoftI2c_Release(bus, bus->sda_pin);
    SoftI2c_Delay(bus);
}

static SoftI2cResult SoftI2c_WriteByte(const SoftI2cBus *bus, uint8_t value)
{
    uint8_t bit;
    SoftI2cResult result;

    for (bit = 0U; bit < 8U; ++bit)
    {
        if ((value & 0x80U) != 0U)
        {
            SoftI2c_Release(bus, bus->sda_pin);
        }
        else
        {
            SoftI2c_DriveLow(bus, bus->sda_pin);
        }
        SoftI2c_Delay(bus);
        result = SoftI2c_ReleaseClock(bus);
        if (result != SOFT_I2C_OK)
        {
            return result;
        }
        SoftI2c_DriveLow(bus, bus->scl_pin);
        SoftI2c_Delay(bus);
        value <<= 1U;
    }

    SoftI2c_Release(bus, bus->sda_pin);
    SoftI2c_Delay(bus);
    result = SoftI2c_ReleaseClock(bus);
    if (result != SOFT_I2C_OK)
    {
        return result;
    }
    if (SoftI2c_IsHigh(bus, bus->sda_pin))
    {
        SoftI2c_DriveLow(bus, bus->scl_pin);
        return SOFT_I2C_ERROR_ACK;
    }
    SoftI2c_DriveLow(bus, bus->scl_pin);
    SoftI2c_Delay(bus);
    return SOFT_I2C_OK;
}

static SoftI2cResult SoftI2c_ReadByte(const SoftI2cBus *bus, uint8_t *value, bool ack)
{
    uint8_t bit;
    SoftI2cResult result;

    *value = 0U;
    SoftI2c_Release(bus, bus->sda_pin);
    for (bit = 0U; bit < 8U; ++bit)
    {
        SoftI2c_Delay(bus);
        result = SoftI2c_ReleaseClock(bus);
        if (result != SOFT_I2C_OK)
        {
            return result;
        }
        *value = (uint8_t)((*value << 1U) | (SoftI2c_IsHigh(bus, bus->sda_pin) ? 1U : 0U));
        SoftI2c_DriveLow(bus, bus->scl_pin);
    }

    if (ack)
    {
        SoftI2c_DriveLow(bus, bus->sda_pin);
    }
    else
    {
        SoftI2c_Release(bus, bus->sda_pin);
    }
    SoftI2c_Delay(bus);
    result = SoftI2c_ReleaseClock(bus);
    SoftI2c_DriveLow(bus, bus->scl_pin);
    SoftI2c_Release(bus, bus->sda_pin);
    SoftI2c_Delay(bus);
    return result;
}

void SoftI2c_Init(SoftI2cBus *bus)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = bus->scl_pin | bus->sda_pin;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(bus->port, &gpio);
    SoftI2c_Release(bus, bus->scl_pin | bus->sda_pin);
    SoftI2c_RecoverBus(bus);
}

SoftI2cResult SoftI2c_RecoverBus(SoftI2cBus *bus)
{
    uint8_t pulse;
    SoftI2cResult result;

    SoftI2c_Release(bus, bus->sda_pin);
    for (pulse = 0U; pulse < 9U; ++pulse)
    {
        SoftI2c_DriveLow(bus, bus->scl_pin);
        SoftI2c_Delay(bus);
        result = SoftI2c_ReleaseClock(bus);
        if (result != SOFT_I2C_OK)
        {
            return result;
        }
        SoftI2c_DriveLow(bus, bus->scl_pin);
    }
    SoftI2c_Stop(bus);
    return SoftI2c_IsHigh(bus, bus->sda_pin) ? SOFT_I2C_OK : SOFT_I2C_ERROR_BUS_BUSY;
}

SoftI2cResult SoftI2c_Write(SoftI2cBus *bus, uint8_t address_7bit,
                             const uint8_t *data, uint16_t length)
{
    SoftI2cResult result;
    uint16_t index;

    result = SoftI2c_Start(bus);
    if (result == SOFT_I2C_OK)
    {
        result = SoftI2c_WriteByte(bus, (uint8_t)(address_7bit << 1U));
    }
    for (index = 0U; (index < length) && (result == SOFT_I2C_OK); ++index)
    {
        result = SoftI2c_WriteByte(bus, data[index]);
    }
    SoftI2c_Stop(bus);
    return result;
}

SoftI2cResult SoftI2c_ReadRegisters(SoftI2cBus *bus, uint8_t address_7bit,
                                     uint8_t reg, uint8_t *data, uint16_t length)
{
    SoftI2cResult result;
    uint16_t index;

    result = SoftI2c_Start(bus);
    if (result == SOFT_I2C_OK)
    {
        result = SoftI2c_WriteByte(bus, (uint8_t)(address_7bit << 1U));
    }
    if (result == SOFT_I2C_OK)
    {
        result = SoftI2c_WriteByte(bus, reg);
    }
    if (result == SOFT_I2C_OK)
    {
        result = SoftI2c_Start(bus);
    }
    if (result == SOFT_I2C_OK)
    {
        result = SoftI2c_WriteByte(bus, (uint8_t)((address_7bit << 1U) | 1U));
    }
    for (index = 0U; (index < length) && (result == SOFT_I2C_OK); ++index)
    {
        result = SoftI2c_ReadByte(bus, &data[index], index + 1U < length);
    }
    SoftI2c_Stop(bus);
    return result;
}
