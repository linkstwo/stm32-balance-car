#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "adc_battery.h"

#define ADC_BATTERY_EOC_TIMEOUT 100000U

void AdcBattery_Init(void)
{
    GPIO_InitTypeDef gpio;
    ADC_InitTypeDef adc;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    gpio.GPIO_Pin = GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio);

    ADC_StructInit(&adc);
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1U;
    ADC_Init(ADC1, &adc);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1U, ADC_SampleTime_239Cycles5);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) != RESET)
    {
    }
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) != RESET)
    {
    }
}

bool AdcBattery_ReadRaw(uint16_t *raw_value)
{
    uint32_t timeout = ADC_BATTERY_EOC_TIMEOUT;

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while ((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) && (timeout-- > 0U))
    {
    }
    if (timeout == 0U)
    {
        return false;
    }
    *raw_value = ADC_GetConversionValue(ADC1);
    return true;
}
