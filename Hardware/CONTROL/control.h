#ifndef __CONTROL_H
#define __CONTROL_H

#include "sys.h"

int Vertical_PD(float measure, float Gyro);
int Velocity_PI(int Speed_measure);
int Turn(int gyro_Z);
void Control_Reset(void);
u8 Control_HasMpuFault(void);
void EXTI9_5_IRQHandler(void);



#endif

