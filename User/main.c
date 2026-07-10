#include "sys.h" 

float Target_Speed = 0;						//期望速度
volatile short gyrox,gyroy,gyroz;					//角速度
volatile float Pitch,Roll,Yaw;						//角度
int Encoder_Left,Encoder_Right;				//编码器速度
u8 Flag_Stop = 0;
volatile u8 Flag_jingzhi,Flag_front,Flag_back,Flag_Left,Flag_Right,Flag_Turn_jingzhi,Speed_Times = 5; //遥控相关变量

int main(void)
{
	u8 mpu_init_ret;
	u8 dmp_init_ret;
	u8 init_err;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//4绾ф姠鍗?4绾у搷搴斻€?
	uart3_init(9600);    //USART3锛氳摑鐗?閬ユ帶鎸囦护鎺ユ敹
	USART2_Init(115200);  //USART2锛歱rintf璋冭瘯杈撳嚭锛屽繀椤诲垵濮嬪寲鍚巔rintf鎵嶄細鐢熸晥

	delay_init();
	NVIC_Config();
	Button_Init();

	OLED_Init();
	OLED_Clear();

	Motor_Init();
	PWM_Init_TIM1(7199,0);
	Motor_Stop();

	mpu_init_ret = MPU_Init();
	dmp_init_ret = 0;
	if(mpu_init_ret == 0)
	{
		dmp_init_ret = mpu_dmp_init();
	}

	init_err = (mpu_init_ret != 0) ? mpu_init_ret : dmp_init_ret;
	if(init_err == 0)
	{
		Encoder_TIM2_Init();
		Encoder_TIM4_Init();
		MPU6050_EXTI_Init();
		printf("MPU init ok\r\n");
	}
	else
	{
		Motor_Stop();
		printf("MPU init fail: mpu=%u dmp=%u err=%u\r\n", mpu_init_ret, dmp_init_ret, init_err);
		OLED_Clear();
		OLED_ShowString(0,0,(u8 *)"MPU ERR",16);
		OLED_ShowNumber(0,2,init_err,2,16);
	}

	while(1)
	{
		if(init_err == 0)
		{
			OLED_Float(0,0,Pitch,1);
		}
		else
		{
			OLED_ShowString(0,0,(u8 *)"MPU ERR",16);
			OLED_ShowNumber(0,2,init_err,2,16);
		}
		delay_ms(50);
	}
}