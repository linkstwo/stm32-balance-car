#include <stdint.h>
#include "stm32f10x.h"
#include "board_config.h"
#include "encoder.h"

#define ENCODER_INPUT_FILTER 10U

static void Encoder_ConfigureTimer(TIM_TypeDef *timer)
{
    TIM_TimeBaseInitTypeDef timebase;
    TIM_ICInitTypeDef input_capture;

    TIM_TimeBaseStructInit(&timebase);
    timebase.TIM_Period = 0xFFFFU;
    timebase.TIM_Prescaler = 0U;
    timebase.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(timer, &timebase);
    TIM_EncoderInterfaceConfig(timer, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&input_capture);
    input_capture.TIM_ICFilter = ENCODER_INPUT_FILTER;
    TIM_ICInit(timer, &input_capture);
    TIM_SetCounter(timer, 0U);
    TIM_Cmd(timer, ENABLE);
}

void Encoder_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM4, ENABLE);

    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &gpio);

    Encoder_ConfigureTimer(TIM2);
    Encoder_ConfigureTimer(TIM4);
}

EncoderDeltaCounts Encoder_ReadDeltaCounts(void)
{
    EncoderDeltaCounts result;
    uint32_t primask = __get_PRIMASK();
    int16_t left_raw;
    int16_t right_raw;

    __disable_irq();
    left_raw = (int16_t)TIM_GetCounter(TIM2);
    right_raw = (int16_t)TIM_GetCounter(TIM4);
    TIM_SetCounter(TIM2, 0U);
    TIM_SetCounter(TIM4, 0U);
    if (primask == 0U)
    {
        __enable_irq();
    }

    result.left_delta_counts = (int16_t)(left_raw * BOARD_ENCODER_LEFT_SIGN);
    result.right_delta_counts = (int16_t)(right_raw * BOARD_ENCODER_RIGHT_SIGN);
    return result;
}

float Encoder_CountsPerSecond(int16_t delta_counts, uint32_t sample_period_ms)
{
    if (sample_period_ms == 0U)
    {
        return 0.0f;
    }
    return ((float)delta_counts * 1000.0f) / (float)sample_period_ms;
}
