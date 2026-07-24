#ifndef BSP_TIMEBASE_H
#define BSP_TIMEBASE_H

#include <stdbool.h>
#include <stdint.h>

void Timebase_Init(void);
void Timebase_OnTimerIrq(void);
uint32_t Timebase_GetMs(void);
void Timebase_DelayUs(uint32_t delay_us);
void Timebase_DelayMs(uint32_t delay_ms);
bool Timebase_HasElapsed(uint32_t now_ms, uint32_t start_ms, uint32_t timeout_ms);

#endif
