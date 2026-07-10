#include "delay.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"
#endif

static u8 fac_us = 0;
static u16 fac_ms = 0;
static volatile u32 delay_ms_tick = 0;

static void delay_tick_init(void)
{
    RCC_ClocksTypeDef clocks;
    TIM_TimeBaseInitTypeDef tim;
    NVIC_InitTypeDef nvic;
    u32 tim_clock;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_GetClocksFreq(&clocks);
    tim_clock = clocks.PCLK1_Frequency;
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
    {
        tim_clock *= 2U;
    }

    TIM_TimeBaseStructInit(&tim);
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Prescaler = (u16)(tim_clock / 10000U - 1U);
    tim.TIM_Period = 9;
    TIM_TimeBaseInit(TIM3, &tim);

    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    nvic.NVIC_IRQChannel = TIM3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 3;
    nvic.NVIC_IRQChannelSubPriority = 3;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        delay_ms_tick++;
    }
}

u32 delay_get_ms(void)
{
    return delay_ms_tick;
}

#if SYSTEM_SUPPORT_OS
#ifdef OS_CRITICAL_METHOD
#define delay_osrunning      OSRunning
#define delay_ostickspersec  OS_TICKS_PER_SEC
#define delay_osintnesting   OSIntNesting
#endif

#ifdef CPU_CFG_CRITICAL_METHOD
#define delay_osrunning      OSRunning
#define delay_ostickspersec  OSCfg_TickRate_Hz
#define delay_osintnesting   OSIntNestingCtr
#endif

void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD
    OS_ERR err;
    OSSchedLock(&err);
#else
    OSSchedLock();
#endif
}

void delay_osschedunlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD
    OS_ERR err;
    OSSchedUnlock(&err);
#else
    OSSchedUnlock();
#endif
}

void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
    OS_ERR err;
    OSTimeDly(ticks, OS_OPT_TIME_PERIODIC, &err);
#else
    OSTimeDly(ticks);
#endif
}

void SysTick_Handler(void)
{
    if (delay_osrunning == 1)
    {
        OSIntEnter();
        OSTimeTick();
        OSIntExit();
    }
}
#endif

void delay_init(void)
{
#if SYSTEM_SUPPORT_OS
    u32 reload;
#endif

    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    fac_us = SystemCoreClock / 8000000;
    delay_tick_init();
#if SYSTEM_SUPPORT_OS
    reload = SystemCoreClock / 8000000;
    reload *= 1000000 / delay_ostickspersec;
    fac_ms = 1000 / delay_ostickspersec;

    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = reload;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
#else
    fac_ms = (u16)fac_us * 1000;
#endif
}

#if SYSTEM_SUPPORT_OS
void delay_us(u32 nus)
{
    u32 ticks;
    u32 told;
    u32 tnow;
    u32 tcnt = 0;
    u32 reload = SysTick->LOAD;

    ticks = nus * fac_us;
    delay_osschedlock();
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
    delay_osschedunlock();
}

void delay_ms(u16 nms)
{
    if (delay_osrunning && delay_osintnesting == 0)
    {
        if (nms >= fac_ms)
        {
            delay_ostimedly(nms / fac_ms);
        }
        nms %= fac_ms;
    }
    delay_us((u32)(nms * 1000));
}
#else
void delay_us(u32 nus)
{
    u32 temp;

    SysTick->LOAD = nus * fac_us;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}

void delay_ms(u16 nms)
{
    u32 temp;

    SysTick->LOAD = (u32)nms * fac_ms;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}
#endif
