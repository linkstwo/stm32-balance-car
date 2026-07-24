#include "stm32f10x_it.h"
#include "timebase.h"
#include "uart3.h"
#include "imu.h"

void NMI_Handler(void) { }
void HardFault_Handler(void) { while (1) { } }
void MemManage_Handler(void) { while (1) { } }
void BusFault_Handler(void) { while (1) { } }
void UsageFault_Handler(void) { while (1) { } }
void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }
void SysTick_Handler(void) { }

void TIM3_IRQHandler(void)
{
    Timebase_OnTimerIrq();
}

void USART3_IRQHandler(void)
{
    Uart3_OnIrq();
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line5);
        Imu_NotifyDataReadyFromIsr(Timebase_GetMs());
    }
}
