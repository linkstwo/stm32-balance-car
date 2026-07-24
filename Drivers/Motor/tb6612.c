#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "board_config.h"
#include "motor_pwm.h"
#include "tb6612.h"

static int16_t MotorDriver_Clamp(int16_t value)
{
    int16_t limit = (int16_t)MotorPwm_GetPeriod();

    if (value > limit)
    {
        return limit;
    }
    if (value < -limit)
    {
        return (int16_t)-limit;
    }
    return value;
}

static uint16_t MotorDriver_Absolute(int16_t value)
{
    return (value < 0) ? (uint16_t)(-value) : (uint16_t)value;
}

static void MotorDriver_SetDirection(uint16_t pin_a, uint16_t pin_b, int16_t output,
                                     int forward_a_high)
{
    bool a_high;

    if (output == 0)
    {
        GPIO_ResetBits(GPIOB, pin_a | pin_b);
        return;
    }
    a_high = ((output > 0) && (forward_a_high != 0)) ||
             ((output < 0) && (forward_a_high == 0));
    if (a_high)
    {
        GPIO_SetBits(GPIOB, pin_a);
        GPIO_ResetBits(GPIOB, pin_b);
    }
    else
    {
        GPIO_ResetBits(GPIOB, pin_a);
        GPIO_SetBits(GPIOB, pin_b);
    }
}

void MotorDriver_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);
    MotorPwm_Init();
    MotorDriver_Coast();
}

void MotorDriver_Set(int16_t left, int16_t right)
{
    int16_t m1 = (int16_t)(MotorDriver_Clamp(left) * BOARD_MOTOR_LEFT_DIRECTION_SIGN);
    int16_t m2 = (int16_t)(MotorDriver_Clamp(right) * BOARD_MOTOR_RIGHT_DIRECTION_SIGN);

    MotorDriver_SetDirection(GPIO_Pin_14, GPIO_Pin_15, m1,
                             BOARD_MOTOR_LEFT_FORWARD_A_HIGH);
    MotorDriver_SetDirection(GPIO_Pin_13, GPIO_Pin_12, m2,
                             BOARD_MOTOR_RIGHT_FORWARD_A_HIGH);
    MotorPwm_SetDuty(MotorDriver_Absolute(m1), MotorDriver_Absolute(m2));
}

void MotorDriver_Coast(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    MotorPwm_SetDuty(0U, 0U);
}

void MotorDriver_Brake(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    MotorPwm_SetDuty(0U, 0U);
}

void MotorDriver_Disable(void)
{
    MotorDriver_Coast();
}

int16_t MotorDriver_GetOutputLimit(void)
{
    return (int16_t)MotorPwm_GetPeriod();
}
