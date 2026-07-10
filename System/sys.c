#include "sys.h" 


void NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	//MPU6050中断
	NVIC_InitStruct.NVIC_IRQChannel=EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	//串口3中断
	NVIC_InitStruct.NVIC_IRQChannel=USART3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStruct);	
	//串口二中断：用于printf调试输出
	NVIC_InitStruct.NVIC_IRQChannel=USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStruct);
}
