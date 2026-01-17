#include "openmv.h"

void clear_RxBuffer(u8 *buffer);

void USART3_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE );
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
//	USART_Cmd(USART3, ENABLE);                    //使能串口 
}

int theta, rho;

void USART3_IRQHandler(void) 
{
	char a;
	uint8_t com_data; //接收一个字节数据的临时变量
	static uint8_t RxCounter1=0;//数据缓冲区的索引
    static uint8_t RxBuffer1[10]={0};//存放数据的接收缓存区

	if( USART_GetITStatus(USART3 ,USART_IT_RXNE) != RESET)  	   //接收中断  
	{
		USART_ClearITPendingBit(USART3 ,USART_IT_RXNE);   //清除中断标志
		com_data = USART_ReceiveData(USART3);
		if(com_data == ']')
		{
			if(RxBuffer1[0] == '[')
			{
				sscanf((const char*)RxBuffer1, "%c%d,%d", &a, &theta, &rho);
				if(theta > 100)
				{
					theta = -(theta - 100);
				}
				if(rho > 100)
				{
					rho = -(rho - 100);
				}
//				printf("rheta:%d,  rho:%d\n", theta, rho);
				if(theta > 0){Flag_OpenMv_right = 1;Flag_OpenMv_left = 0;}
				if(theta > 0){Flag_OpenMv_right = 0;Flag_OpenMv_left = 1;}
				if(rho > 0) {Flag_OpenMv_right = 1;Flag_OpenMv_left = 0;}
				if(rho < 0){Flag_OpenMv_right = 0;Flag_OpenMv_left = 1;}
				clear_RxBuffer(RxBuffer1);
			}
			RxCounter1 = 0;
		}
		else
		{
			RxBuffer1[RxCounter1 ++] = com_data;
		}
		if(RxCounter1 > 10)
		{
			RxCounter1 = 0;
			clear_RxBuffer(RxBuffer1);
		}
	}
	
}

void clear_RxBuffer(u8 *buffer)
{
	u8 i;
	for(i = 0; i < 10; i ++)
	{
		buffer[i] = '\0';
	}
}








