#include "control.h"

#define MPU_FAIL_LIMIT 5

float Med_Angle = -8;
float Vertical_Kp = -330,
      Vertical_Kd = -2.2;

float Velocity_Kp = 120,
      Velocity_Ki = 0.6;

float Turn_Kp = 40,
      Turn_Kd = -0.8;

int Vertical_out, Velocity_out, Turn_out;
int measure;
int motor1, motor2;
static int Encoder_err;
static int Encoder_err_low;
static int Encoder_err_low_last;
static int Encoder_sum;
static int Movement;
static float Turn_Target;
static volatile u8 mpu_read_fail_count = 0;
static volatile u8 mpu_fault_latched = 0;

int Vertical_PD(float measure, float Gyro);
int Velocity_PI(int Speed_measure);
int Turn(int gyro_Z);

void Control_Reset(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    Encoder_err = 0;
    Encoder_err_low = 0;
    Encoder_err_low_last = 0;
    Encoder_sum = 0;
    Movement = 0;
    Turn_Target = 0;
    Vertical_out = 0;
    Velocity_out = 0;
    Turn_out = 0;
    measure = 0;
    motor1 = 0;
    motor2 = 0;
    Encoder_Left = 0;
    Encoder_Right = 0;
    if(primask == 0U)
    {
        __enable_irq();
    }
}

u8 Control_HasMpuFault(void)
{
    return mpu_fault_latched;
}

static void Control_EnterStaticState(void)
{
    Flag_front = 0;
    Flag_back = 0;
    Flag_Left = 0;
    Flag_Right = 0;
    Flag_jingzhi = 1;
    Flag_Turn_jingzhi = 1;
    Motor_Stop();
    Control_Reset();
}

void EXTI9_5_IRQHandler(void)
{
    int PWM_out;
    float pitch, roll, yaw;
    short gyro_x, gyro_y, gyro_z;
    u8 dmp_ret, gyro_ret;

    if(EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line5);
        if(PBin(5) == 0)
        {
            if(mpu_fault_latched == 1)
            {
                Control_EnterStaticState();
                return;
            }

            Encoder_Left = Read_Spead(2);
            Encoder_Right = -Read_Spead(4);
            measure = (Encoder_Left + Encoder_Right);

            dmp_ret = mpu_dmp_get_data(&pitch, &roll, &yaw);
            gyro_ret = MPU_Get_Gyroscope(&gyro_x, &gyro_y, &gyro_z);
            if(dmp_ret != 0 || gyro_ret != 0)
            {
                if(mpu_read_fail_count < MPU_FAIL_LIMIT)
                {
                    mpu_read_fail_count++;
                }
                Control_EnterStaticState();
                if(mpu_read_fail_count >= MPU_FAIL_LIMIT)
                {
                    mpu_fault_latched = 1;
                }
                return;
            }

            mpu_read_fail_count = 0;
            Pitch = pitch;
            Roll = roll;
            Yaw = yaw;
            gyrox = gyro_x;
            gyroy = gyro_y;
            gyroz = gyro_z;

            Vertical_out = Vertical_PD(Pitch, gyroy);
            Velocity_out = Velocity_PI(measure);
            Turn_out = Turn(gyroz);

            PWM_out = Vertical_out + Velocity_out;
            motor1 = PWM_out - Turn_out;
            motor2 = PWM_out + Turn_out;

            Limit(&motor1, &motor2);
            if(Turn_Off(Pitch) == 0)
            {
                SETPWM(motor1, motor2);
            }
            else
            {
                Control_EnterStaticState();
            }
        }
    }
}

int Vertical_PD(float measure, float Gyro)
{
    int PWM_out;

    PWM_out = Vertical_Kp*(measure - Med_Angle) + Vertical_Kd*Gyro;
    return PWM_out;
}

int Velocity_PI(int Speed_measure)
{
    int PWM_out;
    const float Target_Velocity = 300;

    if(Flag_front == 1)         Movement = Target_Velocity / Speed_Times;
    else if(Flag_back == 1)     Movement = -Target_Velocity / Speed_Times;
    else if(Flag_jingzhi == 1)  Movement = 0;
    else                        Movement = 0;

    Encoder_err = 0 - Speed_measure;
    Encoder_err_low = 0.3 * Encoder_err + 0.7 * Encoder_err_low_last;
    Encoder_err_low_last = Encoder_err_low;

    Encoder_sum += Encoder_err_low;
    Encoder_sum = Encoder_sum + Movement;
    if(Encoder_sum > 10000) Encoder_sum = 10000;
    if(Encoder_sum < -10000) Encoder_sum = -10000;

    PWM_out = Velocity_Kp*Encoder_err + Velocity_Ki*Encoder_sum;

    if(Turn_Off(Pitch) == 1) Encoder_sum = 0;

    return PWM_out;
}

int Turn(int gyro_Z)
{
    int PWM_out;
    const float Turn_Amplitude = 30;
    float Kp = Turn_Kp, Kd;

    if(1 == Flag_Left)              Turn_Target = -Turn_Amplitude/2;
    else if(1 == Flag_Right)        Turn_Target = Turn_Amplitude/2;
    else if(1 == Flag_Turn_jingzhi) Turn_Target = 0;
    else                            Turn_Target = 0;

    if(1 == Flag_Left || 1 == Flag_Right) Kd = 0;
    else                                  Kd = Turn_Kd;

    PWM_out = Turn_Target * Kp + (gyro_Z + 13) * Kd;

    return PWM_out;
}
