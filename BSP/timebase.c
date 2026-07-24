#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "timebase.h"

static volatile uint32_t g_time_ms;

void Timebase_Init(void)
{
    RCC_ClocksTypeDef clocks;
    TIM_TimeBaseInitTypeDef timer;
    uint32_t timer_clock_hz;

    RCC_GetClocksFreq(&clocks);
    timer_clock_hz = clocks.PCLK1_Frequency;
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
    {
        timer_clock_hz *= 2U;
    }

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = (uint16_t)(timer_clock_hz / 1000000U - 1U);
    timer.TIM_Period = 999U;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &timer);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

}

void Timebase_OnTimerIrq(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        g_time_ms++;
    }
}

uint32_t Timebase_GetMs(void)
{
    return g_time_ms;
}

void Timebase_DelayUs(uint32_t delay_us)
{
    while (delay_us > 0U)
    {
        uint16_t start = TIM_GetCounter(TIM3);
        uint16_t step = (delay_us > 500U) ? 500U : (uint16_t)delay_us;
        uint16_t elapsed;

        do
        {
            uint16_t current = TIM_GetCounter(TIM3);
            elapsed = (current >= start) ? (uint16_t)(current - start) :
                      (uint16_t)(1000U - start + current);
        } while (elapsed < step);
        delay_us -= step;
    }
}

void Timebase_DelayMs(uint32_t delay_ms)
{
    while (delay_ms-- > 0U)
    {
        Timebase_DelayUs(1000U);
    }
}

bool Timebase_HasElapsed(uint32_t now_ms, uint32_t start_ms, uint32_t timeout_ms)
{
    return (uint32_t)(now_ms - start_ms) >= timeout_ms;
}
