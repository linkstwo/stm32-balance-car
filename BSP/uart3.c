#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "timebase.h"
#include "uart3.h"

#define UART3_RX_BUFFER_SIZE 64U

static volatile uint8_t g_rx_buffer[UART3_RX_BUFFER_SIZE];
static volatile uint8_t g_rx_head;
static volatile uint8_t g_rx_tail;
static volatile uint32_t g_last_receive_ms;
static volatile uint32_t g_dropped_bytes;

void Uart3_Init(uint32_t baud)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate = baud;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &usart);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

void Uart3_OnIrq(void)
{
    uint8_t next_head;
    uint8_t byte;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) == RESET)
    {
        return;
    }

    byte = (uint8_t)USART_ReceiveData(USART3);
    next_head = (uint8_t)((g_rx_head + 1U) % UART3_RX_BUFFER_SIZE);
    if (next_head == g_rx_tail)
    {
        g_dropped_bytes++;
    }
    else
    {
        g_rx_buffer[g_rx_head] = byte;
        g_rx_head = next_head;
    }
    g_last_receive_ms = Timebase_GetMs();
}

bool Uart3_ReadByte(uint8_t *byte)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    if (g_rx_tail == g_rx_head)
    {
        if (primask == 0U)
        {
            __enable_irq();
        }
        return false;
    }
    *byte = g_rx_buffer[g_rx_tail];
    g_rx_tail = (uint8_t)((g_rx_tail + 1U) % UART3_RX_BUFFER_SIZE);
    if (primask == 0U)
    {
        __enable_irq();
    }
    return true;
}

uint32_t Uart3_GetLastReceiveMs(void)
{
    return g_last_receive_ms;
}

uint32_t Uart3_GetDroppedByteCount(void)
{
    return g_dropped_bytes;
}
