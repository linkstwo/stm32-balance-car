#include "sys.h" 

float Target_Speed = 0;						//期望速度
short gyrox,gyroy,gyroz;					//角速度
float Pitch,Roll,Yaw;						//角度
int Encoder_Left,Encoder_Right;				//编码器速度
u8 Flag_Stop = 0;
u8 Flag_jingzhi,Flag_front,Flag_back,Flag_Left,Flag_Right,Flag_Turn_jingzhi,Speed_Times = 5; //遥控相关变量

int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//4级抢占,4级响应。
	uart3_init(9600);			//USART3：蓝牙/遥控指令接收
	USART2_Init(115200);		//USART2：printf调试输出，必须初始化后printf才会生效
	
	delay_init();
	NVIC_Config();
	Button_Init();
	
	OLED_Init();
	OLED_Clear();
	
	MPU_Init();
	mpu_dmp_init();
	
	Encoder_TIM2_Init();
	Encoder_TIM4_Init();
	Motor_Init();
	PWM_Init_TIM1(7199,0);
	
	MPU6050_EXTI_Init();
	printf("初始化完成\n");
	while(1)
	{
		OLED_Float(0,0,Pitch,1);
	}
}
