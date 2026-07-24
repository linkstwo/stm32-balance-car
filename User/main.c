#include "sys.h"
#include "board.h"

float Target_Speed = 0;                                      // target speed
volatile short gyrox, gyroy, gyroz;                          // gyro speed
volatile float Pitch, Roll, Yaw;                             // attitude
int Encoder_Left, Encoder_Right;                             // encoder speed
u8 Flag_Stop = 0;
volatile u8 Flag_jingzhi = 1;
volatile u8 Flag_front = 0;
volatile u8 Flag_back = 0;
volatile u8 Flag_Left = 0;
volatile u8 Flag_Right = 0;
volatile u8 Flag_Turn_jingzhi = 1;
volatile u8 Speed_Times = 5;                                 // remote speed level

static void ClearMotionFlags(void)
{
    Flag_front = 0;
    Flag_back = 0;
    Flag_Left = 0;
    Flag_Right = 0;
}

static void EnterStaticState(void)
{
    ClearMotionFlags();
    Flag_jingzhi = 1;
    Flag_Turn_jingzhi = 1;
    Motor_Stop();
    Control_Reset();
}

int main(void)
{
    u8 mpu_init_ret;
    u8 dmp_init_ret;
    u8 init_err;
    u8 mpu_fault_reported = 0;
    u32 last_cmd_ms;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    Board_PinMux_Init();

    delay_init();

    Motor_Init();
    PWM_Init_TIM1(7199, 0);
    EnterStaticState();

    OLED_Init();
    OLED_Clear();

    uart3_init(9600);
    NVIC_Config();

    mpu_init_ret = MPU_Init();
    dmp_init_ret = 0;
    if (mpu_init_ret == 0)
    {
        dmp_init_ret = mpu_dmp_init();
    }

    init_err = (mpu_init_ret != 0) ? mpu_init_ret : dmp_init_ret;
    if (init_err == 0)
    {
        Encoder_TIM2_Init();
        Encoder_TIM4_Init();
        EnterStaticState();
        last_cmd_ms = delay_get_ms();
        Usart3_LastValidCmdMs = last_cmd_ms;
        MPU6050_EXTI_Init();
    }
    else
    {
        EnterStaticState();
        OLED_ShowString(0, 0, (u8 *)"MPU ERR", 16);
        OLED_ShowNumber(0, 2, init_err, 2, 16);
    }

    while (1)
    {
        if (init_err == 0)
        {
            if (Control_HasMpuFault() == 1)
            {
                if (mpu_fault_reported == 0)
                {
                    EnterStaticState();
                    OLED_Clear();
                    OLED_ShowString(0, 0, (u8 *)"MPU LOST", 16);
                    mpu_fault_reported = 1;
                }
            }
            else
            {
                last_cmd_ms = Usart3_LastValidCmdMs;
                if ((u32)(delay_get_ms() - last_cmd_ms) > 300U)
                {
                    EnterStaticState();
                }
                OLED_Float(0, 0, Pitch, 1);
            }
        }
        else
        {
            OLED_ShowString(0, 0, (u8 *)"MPU ERR", 16);
            OLED_ShowNumber(0, 2, init_err, 2, 16);
        }
        delay_ms(50);
    }
}
