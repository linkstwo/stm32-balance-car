#ifndef DRIVERS_IMU_INV_MPU_PORT_H
#define DRIVERS_IMU_INV_MPU_PORT_H

#include <stdint.h>

void InvMpuPort_Init(void);
unsigned char MPU_Write_Len(unsigned char address, unsigned char reg,
                            unsigned char length, const unsigned char *data);
unsigned char MPU_Read_Len(unsigned char address, unsigned char reg,
                           unsigned char length, unsigned char *data);
void delay_ms(unsigned long delay_ms_value);
unsigned long delay_get_ms(void);
void MPU_IIC_Init(void);

#endif
