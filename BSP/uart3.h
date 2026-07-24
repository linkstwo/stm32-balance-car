#ifndef BSP_UART3_H
#define BSP_UART3_H

#include <stdbool.h>
#include <stdint.h>

void Uart3_Init(uint32_t baud);
void Uart3_OnIrq(void);
bool Uart3_ReadByte(uint8_t *byte);
uint32_t Uart3_GetLastReceiveMs(void);
uint32_t Uart3_GetDroppedByteCount(void);

#endif
