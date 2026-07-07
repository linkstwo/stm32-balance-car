#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "sys.h" 

void uart3_init(u32 bound);					//串口3初始化函数
void USART3_IRQHandler(void);	    	//串口3中断服务程序
#endif
