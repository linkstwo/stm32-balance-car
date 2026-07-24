#ifndef BSP_ADC_BATTERY_H
#define BSP_ADC_BATTERY_H

#include <stdbool.h>
#include <stdint.h>

void AdcBattery_Init(void);
bool AdcBattery_ReadRaw(uint16_t *raw_value);

#endif
