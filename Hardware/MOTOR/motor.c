#include "motor.h"

void Motor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	Ain1 = 0;
	Ain2 = 0;
	Bin1 = 0;
	Bin2 = 0;
}

void Limit(int *motoA, int *motoB)
{
	if(*motoA > 7199) *motoA = 7199;
	if(*motoA < -7199) *motoA = -7199;

	if(*motoB > 7199) *motoB = 7199;
	if(*motoB < -7199) *motoB = -7199;
}

int GFP_abs(int p)
{
	int q;
	q = p > 0 ? p : (-p);
	return q;
}

void SETPWM(int motor_left, int motor_right)
{
	if(motor_left > 0)        Ain1 = 1, Ain2 = 0;
	else                      Ain1 = 0, Ain2 = 1;
	TIM_SetCompare1(TIM1, GFP_abs(motor_left));

	if(motor_right > 0)       Bin1 = 0, Bin2 = 1;
	else                      Bin1 = 1, Bin2 = 0;
	TIM_SetCompare4(TIM1, GFP_abs(motor_right));
}

void Motor_Stop(void)
{
	Ain1 = 0;
	Ain2 = 0;
	Bin1 = 0;
	Bin2 = 0;
	TIM_SetCompare1(TIM1, 0);
	TIM_SetCompare4(TIM1, 0);
}

u8 Turn_Off(float angle)
{
	u8 temp;
	if(angle < -50 || angle > 40)
	{
		temp = 1;
		Motor_Stop();
	}
	else
	{
		temp = 0;
	}
	return temp;
}
