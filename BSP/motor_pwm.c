#include <stdint.h>
#include "stm32f10x.h"
#include "board_config.h"
#include "motor_pwm.h"

void MotorPwm_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef timebase;
    TIM_OCInitTypeDef output_compare;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    TIM_TimeBaseStructInit(&timebase);
    timebase.TIM_Prescaler = BOARD_MOTOR_PWM_PRESCALER;
    timebase.TIM_Period = BOARD_MOTOR_PWM_PERIOD;
    timebase.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &timebase);

    TIM_OCStructInit(&output_compare);
    output_compare.TIM_OCMode = TIM_OCMode_PWM1;
    output_compare.TIM_OutputState = TIM_OutputState_Enable;
    output_compare.TIM_Pulse = 0U;
    output_compare.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &output_compare);
    TIM_OC4Init(TIM1, &output_compare);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

void MotorPwm_SetDuty(uint16_t m1_duty, uint16_t m2_duty)
{
    if (m1_duty > BOARD_MOTOR_PWM_PERIOD)
    {
        m1_duty = BOARD_MOTOR_PWM_PERIOD;
    }
    if (m2_duty > BOARD_MOTOR_PWM_PERIOD)
    {
        m2_duty = BOARD_MOTOR_PWM_PERIOD;
    }
    TIM_SetCompare1(TIM1, m1_duty);
    TIM_SetCompare4(TIM1, m2_duty);
}

uint16_t MotorPwm_GetPeriod(void)
{
    return BOARD_MOTOR_PWM_PERIOD;
}
