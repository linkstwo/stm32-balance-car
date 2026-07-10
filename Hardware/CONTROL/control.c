#include "control.h"

float Med_Angle = -8;          //机械中值：车体自然直立时的Pitch角度，需要按实车校准
float Vertical_Kp = -330,      //直立环Kp
      Vertical_Kd = -2.2;      //直立环Kd

float Velocity_Kp = 120,       //速度环Kp
      Velocity_Ki = 0.6;       //速度环Ki

float Turn_Kp = 40,            //转向环Kp
      Turn_Kd = -0.8;          //转向环Kd

int Vertical_out,Velocity_out,Turn_out;     //三环控制输出

extern float Med_Angle;                     //机械中值
int measure;                                //左右编码器速度和
int motor1, motor2;                         //左右电机最终PWM

int Vertical_PD(float measure, float Gyro);
int Velocity_PI(int Speed_measure);
int Turn(int gyro_Z);


void EXTI9_5_IRQHandler(void)
{
    int PWM_out;
    if(EXTI_GetITStatus(EXTI_Line5)!=0)
    {
        if(PBin(5) == 0)
        {
            EXTI_ClearITPendingBit(EXTI_Line5);

            Encoder_Left = Read_Spead(2);
            Encoder_Right = -Read_Spead(4);
            measure = (Encoder_Left + Encoder_Right);

            mpu_dmp_get_data(&Pitch, &Roll, &Yaw);
            MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);

            Vertical_out = Vertical_PD(Pitch, gyroy);
            Velocity_out = Velocity_PI(measure);
            Turn_out = Turn(gyroz);

            PWM_out = Vertical_out + Velocity_out;
            motor1 = PWM_out - Turn_out;
            motor2 = PWM_out + Turn_out;

            Limit(&motor1, &motor2);
            if(Turn_Off(Pitch) == 0)
            {
                SETPWM(motor1,motor2);
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
    static int Encoder_err, Encoder_err_low, Encoder_err_low_last, Encoder_sum, Movement;
    static int PWM_out;
    const float Target_Velocity = 300;                          //遥控模式基础速度

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
    static float Turn_Target;
    const float Turn_Amplitude = 30;                             //遥控转向幅度
    float Kp = Turn_Kp,Kd;

    if(1 == Flag_Left)              Turn_Target = -Turn_Amplitude/2;
    else if(1 == Flag_Right)        Turn_Target = Turn_Amplitude/2;
    else if(1 == Flag_Turn_jingzhi) Turn_Target = 0;
    else                            Turn_Target = 0;

    if(1 == Flag_Left || 1 == Flag_Right) Kd = 0;
    else                                  Kd = Turn_Kd;

    PWM_out = Turn_Target * Kp + (gyro_Z + 13) * Kd;

    return PWM_out;
}
