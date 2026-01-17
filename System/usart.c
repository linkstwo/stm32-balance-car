#include "usart.h"
#include "string.h"
#include "stm32f10x.h"  // 确保包含STM32标准库头文件

// 声明外部全局变量（需在其他文件中定义）
extern u8 Flag_front;    // 前进标志
extern u8 Flag_back;     // 后退标志
extern u8 Flag_Left;     // 左移/左转标志
extern u8 Flag_Right;    // 右移/右转标志
extern u8 Flag_jingzhi;  // 静止标志
extern u8 Flag_Turn_jingzhi; // 转向静止标志
extern u8 Speed_Times;   // 速度倍数


// 若需使用printf，取消以下注释（确保仅在一个文件中定义fputc）
/*
// 使用microLib重定向printf到USART1
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
    return ch;
}

// 串口接收字符（类似getchar）
int GetKey(void)
{
    while (!(USART1->SR & USART_FLAG_RXNE));
    return ((int)(USART1->DR & 0x1FF));
}
*/


u8 USART_RX_BUF[64];     // 接收缓冲区
u8 USART_RX_STA = 0;     // 接收状态标志（bit7:完成, bit6:收到0x0d, bit0~5:长度）
u8 Usart1_Receive;       // 最新接收的字符


void uart1_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;  // 中断优先级配置

    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    // 配置TX引脚（PA9 复用推挽输出）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置RX引脚（PA10 浮空输入）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART1参数
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 配置中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 抢占优先级（根据系统调整）
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;         // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    // 启用USART1（关键修复：之前被注释导致串口不工作）
    USART_Cmd(USART1, ENABLE);
}


void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  // 接收中断
    {
        u8 usart1_receive = USART_ReceiveData(USART1);  // 读取接收数据
        Usart1_Receive = usart1_receive;                // 保存最新字符

        // 重置所有运动标志（默认每次接收指令时先清零）
        Flag_front = 0;
        Flag_back = 0;
        Flag_Left = 0;
        Flag_Right = 0;
        Flag_jingzhi = 0;
        Flag_Turn_jingzhi = 0;

        switch (usart1_receive)
        {
            case 'Z':  // 静止
                Flag_jingzhi = 1;
                Flag_Turn_jingzhi = 1;
                break;
            case 'E':  // 前进
                Flag_front = 1;
                break;
            case 'A':  // 后退
                Flag_back = 1;
                break;
            case 'C':  // 左移
                Flag_Left = 1;
                break;
            case 'B':  // 右移（修复：原逻辑错误，改为右移）
                Flag_Right = 1;
                break;
            case 'G':  // 左转（修复：原逻辑错误，改为左转）
                Flag_Left = 1;
                break;
            case 'H':  // 右转（修复：原逻辑错误，改为右转）
                Flag_Right = 1;
                break;
            case 'X':  // 减速（最低为1）
                Speed_Times = (Speed_Times > 1) ? (Speed_Times - 1) : 1;
                break;
            case 'Y':  // 加速（可根据需求添加上限）
                Speed_Times++;
                // 示例上限：if (Speed_Times > 5) Speed_Times = 5;
                break;
            default:
                break;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);  // 清除中断标志
    }
}
