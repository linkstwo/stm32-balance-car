#ifndef DRIVERS_MOTOR_TB6612_H
#define DRIVERS_MOTOR_TB6612_H

#include <stdint.h>

void MotorDriver_Init(void);
void MotorDriver_Set(int16_t left, int16_t right);
void MotorDriver_Coast(void);
void MotorDriver_Brake(void);
void MotorDriver_Disable(void);
int16_t MotorDriver_GetOutputLimit(void);

#endif
