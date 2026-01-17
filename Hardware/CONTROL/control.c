#include "control.h"

float Med_Angle = -8;		//机械中值
float Vertical_Kp = -330,		//直立环Kp
	  Vertical_Kd = -2.2;		//直立环Kd

float Velocity_Kp = 120,		//速度环Kp
	  Velocity_Ki = 0.6;		//速度环Kd

float Turn_Kp = 40,
	  Turn_Kd = -0.8;

int Vertical_out,Velocity_out,Turn_out;		//输出变量

extern float Med_Angle;						//机械中值
int measure;								//编码器速度测量值
int motor1, motor2;

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
			Encoder_Left = Read_Spead(2);						//采集编码器速度
			Encoder_Right = -Read_Spead(4);
			measure = (Encoder_Left + Encoder_Right);			//将编码器速度给测量值
			mpu_dmp_get_data(&Pitch, &Roll, &Yaw);				//角度
			MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);			//角速度
			Vertical_out = Vertical_PD(Pitch, gyroy);			//直立环计算
			Velocity_out = Velocity_PI(measure);				//速度环计算
			Turn_out = Turn(gyroz);
			
			PWM_out = Vertical_out + Velocity_out;				//PWM输出
			motor1 = PWM_out - Turn_out;
			motor2 = PWM_out + Turn_out;
			Limit(&motor1, &motor2);							//PWM限幅
			if(Turn_Off(Pitch) == 0)
			SETPWM(motor1,motor2);								//加载PWM到电机
			
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
			{
				Flag_OpenMv_left = 0;Flag_OpenMv_right = 0;Flag_back = 0;
				USART_Cmd(USART3, DISABLE);
				USART_Cmd(USART1, ENABLE);
				Flag_yaokong = 1;Flag_xunji = 0;
			}
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)
			{
				Flag_back = 1;
				USART_Cmd(USART1, DISABLE);
				USART_Cmd(USART3, ENABLE);
				Flag_yaokong = 0;Flag_xunji = 1;
			}
		}
	}
}

//直立环PD
int Vertical_PD(float measure, float Gyro)
{
	int PWM_out;
	
	PWM_out = Vertical_Kp*(measure - Med_Angle) + Vertical_Kd*Gyro;
	return PWM_out;
}

//速度环PI
int Velocity_PI(int Speed_measure)
{
	static int Encoder_err, Encoder_err_low, Encoder_err_low_last, Encoder_sum, Movement;
	static int PWM_out;
	static float Target_Velocity;
	
	if(Flag_yaokong == 1)		Target_Velocity = 300;
	else						Target_Velocity= 100;
	if(Flag_front == 1)			Movement = Target_Velocity / Speed_Times;
	else if(Flag_back == 1)    	Movement = -Target_Velocity / Speed_Times;
	else if(Flag_jingzhi == 1)	Movement = 0;

	else Movement = 0;
	
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

//转向环
int Turn(int gyro_Z)
{
	int PWM_out;
	static float Turn_Target , Turn_Amplitude;
	float Kp = Turn_Kp,Kd;			
	if(Flag_yaokong == 1)								Turn_Amplitude = 30;
	else												Turn_Amplitude=12;
	if(1 == Flag_Left || 1 == Flag_OpenMv_left)	        Turn_Target = -Turn_Amplitude/2;
	else if(1 == Flag_Right || 1 == Flag_OpenMv_right)	Turn_Target = Turn_Amplitude/2;
	else if(1 == Flag_Turn_jingzhi)	 Turn_Target = 0;
	else Turn_Target = 0;
	
	if(1 == Flag_Left || 1 == Flag_Right || Flag_OpenMv_left || Flag_OpenMv_right)   Kd = 0;       
	else   Kd = Turn_Kd; 
	
	PWM_out = Turn_Target * Kp + (gyro_Z + 13) * Kd;
	
	return PWM_out;
}


