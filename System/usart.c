#include "usart.h"
#include "stm32f10x.h"

extern u8 Flag_front;
extern u8 Flag_back;
extern u8 Flag_Left;
extern u8 Flag_Right;
extern u8 Flag_jingzhi;
extern u8 Flag_Turn_jingzhi;
extern u8 Speed_Times;

u8 Usart3_Receive;

void uart3_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        u8 usart3_receive = USART_ReceiveData(USART3);
        Usart3_Receive = usart3_receive;

        Flag_front = 0;
        Flag_back = 0;
        Flag_Left = 0;
        Flag_Right = 0;
        Flag_jingzhi = 0;
        Flag_Turn_jingzhi = 0;

        switch (usart3_receive)
        {
            case 'Z':
                Flag_jingzhi = 1;
                Flag_Turn_jingzhi = 1;
                break;
            case 'E':
                Flag_front = 1;
                break;
            case 'A':
                Flag_back = 1;
                break;
            case 'C':
                Flag_Left = 1;
                break;
            case 'B':
                Flag_Right = 1;
                break;
            case 'G':
                Flag_Left = 1;
                break;
            case 'H':
                Flag_Right = 1;
                break;
            case 'X':
                Speed_Times = (Speed_Times < 10) ? (Speed_Times + 1) : 10;
                break;
            case 'Y':
                Speed_Times = (Speed_Times > 1) ? (Speed_Times - 1) : 1;
                break;
            default:
                break;
        }
    }
}
