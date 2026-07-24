#ifndef BSP_MOTOR_PWM_H
#define BSP_MOTOR_PWM_H

#include <stdint.h>

void MotorPwm_Init(void);
void MotorPwm_SetDuty(uint16_t m1_duty, uint16_t m2_duty);
uint16_t MotorPwm_GetPeriod(void);

#endif
