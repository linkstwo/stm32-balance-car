#ifndef BSP_SOFT_I2C_H
#define BSP_SOFT_I2C_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"

typedef enum
{
    SOFT_I2C_OK = 0,
    SOFT_I2C_ERROR_BUS_BUSY,
    SOFT_I2C_ERROR_ACK,
    SOFT_I2C_ERROR_CLOCK_STRETCH
} SoftI2cResult;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t scl_pin;
    uint16_t sda_pin;
    uint32_t half_period_us;
} SoftI2cBus;

void SoftI2c_Init(SoftI2cBus *bus);
SoftI2cResult SoftI2c_RecoverBus(SoftI2cBus *bus);
SoftI2cResult SoftI2c_Write(SoftI2cBus *bus, uint8_t address_7bit,
                             const uint8_t *data, uint16_t length);
SoftI2cResult SoftI2c_ReadRegisters(SoftI2cBus *bus, uint8_t address_7bit,
                                     uint8_t reg, uint8_t *data, uint16_t length);

#endif
