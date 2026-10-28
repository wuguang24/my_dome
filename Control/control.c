#include "control.h"

#include "uart_callback.h"

#include <math.h>

u8 CCD_count,ELE_count;
int Sensor_Left,Sensor_Middle,Sensor_Right,Sensor;

volatile uint32_t Control_Loop_Ticks = 0;

volatile uint8_t Avoid_Scan_Request = 0;
volatile uint8_t Avoid_Active = 0;
volatile uint8_t Avoid_Scan_Done = 0;
volatile uint8_t Avoid_TurnDir = 0;

Encoder OriginalEncoder; 					//编码器原始数据   
Motor_parameter MotorA,MotorB;				//左右电机相关变量
float Velocity_KP=400,Velocity_KI=300;	
int Run_Mode=1;//小车运行模式
u8 Flag_Stop=1;//小车启动标志位

volatile float Odom_X = 0.0f;
volatile float Odom_Y = 0.0f;
volatile float Odom_Theta = 0.0f;
volatile float Odom_V = 0.0f;
volatile float Odom_W = 0.0f;
volatile uint8_t Odom_Enable = 1;
volatile uint8_t BT_Obstacle_Near = 0;

static void odom_reset(void)
{
	Odom_X = 0.0f;
	Odom_Y = 0.0f;
	Odom_Theta = 0.0f;
	Odom_V = 0.0f;
	Odom_W = 0.0f;
}

static float gPI_LeftPwm = 0.0f;
static float gPI_LeftLastBias = 0.0f;
static float gPI_RightPwm = 0.0f;
static float gPI_RightLastBias = 0.0f;

static void VelocityPI_Reset(void)
{
	gPI_LeftPwm = 0.0f;
	gPI_LeftLastBias = 0.0f;
	gPI_RightPwm = 0.0f;
	gPI_RightLastBias = 0.0f;
}

static float odom_wrap_pi(float a)
{
	while (a > PI)  a -= 2.0f * PI;
	while (a < -PI) a += 2.0f * PI;
	return a;
}

static float odom_deg_to_rad(float d)
{
	return d * (PI / 180.0f);
}

static float odom_yaw_from_mpu_rad(void)
{
	/* mpu6050.yaw is updated by DMP interrupt. Unit is assumed degrees per reference demo prints.
	 * If your yaw unit is already rad, switch this conversion.
	 */
	return odom_deg_to_rad(mpu6050.yaw);
}

static void odom_update_200hz(void)
{
	// 优化：刹车时暂时禁用惯性导航，优先保证刹车响应
	if (!Odom_Enable) {
		return;
	}
	
	const float dt = 1.0f / Frequency; /* 0.005s */
	const float v_l = MotorA.Current_Encoder; /* m/s */
	const float v_r = MotorB.Current_Encoder; /* m/s */
	const float v = 0.5f * (v_l + v_r);
	const float w_enc = (v_r - v_l) / Wheelspacing; /* rad/s */
	float theta = Odom_Theta;

	Odom_V = v;
	Odom_W = w_enc;

	/* Predict theta from wheel odom */
	theta = odom_wrap_pi(theta + w_enc * dt);

	/* Complementary correction using MPU yaw (weak, to keep long-term stability from odom) */
	{
		const float yaw_mpu = odom_yaw_from_mpu_rad();
		const float k_yaw = 0.02f; /* 0..1: larger means trust MPU more */
		float err = odom_wrap_pi(yaw_mpu - theta);
		theta = odom_wrap_pi(theta + k_yaw * err);
	}

	/* Update position using mid-point integration */
	{
		const float theta_mid = odom_wrap_pi(Odom_Theta + 0.5f * (theta - Odom_Theta));
		Odom_X += v * dt * (float)cosf(theta_mid);
		Odom_Y += v * dt * (float)sinf(theta_mid);
	}
	Odom_Theta = theta;
}

void TIMER_0_INST_IRQHandler(void)
{
    if(DL_TimerA_getPendingInterrupt(TIMER_0_INST))
    {
        if(DL_TIMER_IIDX_ZERO)
        {
			Control_Loop_Ticks++;
			
			Key();
			LED_Task();
			Get_Velocity_From_Encoder(Get_Encoder_countA,Get_Encoder_countB);
			Get_Encoder_countA=Get_Encoder_countB=0;
			odom_update_200hz();
			if(Run_Mode==0)
			{
				/* Clear stale BT commands if link is silent (prevents run-away after connect) */
				{
					const uint32_t now_ms = (uint32_t)tick_ms;
					/* Some APPs do not continuously stream direction bytes while holding joystick.
					 * Use a slightly longer timeout to avoid Move_X being cleared intermittently.
					 */
					if (BT_LastRxMs == 0U || (uint32_t)(now_ms - BT_LastRxMs) > 300U) {
						Flag_Direction = 0;
						Flag_Left = 0;
						Flag_Right = 0;
						Turn_Flag = 0;
						Move_X = 0.0f;
						Move_Z = 0.0f;
						Get_Target_Encoder(0.0f, 0.0f);
					}
				}
				Get_RC();         //Handle the APP remote commands //处理APP遥控命令
		}else if(Run_Mode==1){
			IRDM_line_inspection();
			Get_Target_Encoder(Move_X,Move_Z);
		}

		/* 蓝牙/巡线模式下，舵机跟随转向摆头 */
		if (Run_Mode == 0 || Run_Mode == 1) {
			float servo_deg = 90.0f + Move_Z * 40.0f;
			if (servo_deg < 30.0f) servo_deg = 30.0f;
			if (servo_deg > 150.0f) servo_deg = 150.0f;
			Servo_SetAngleDeg(servo_angle_offset((uint8_t)servo_deg));
		}

//			//计算左右电机对应的PWM
			MotorA.Motor_Pwm = Incremental_PI_Left(MotorA.Current_Encoder,MotorA.Target_Encoder);	
			MotorB.Motor_Pwm = Incremental_PI_Right(MotorB.Current_Encoder,MotorB.Target_Encoder);
			if(!Flag_Stop)
			{
				Set_PWM(MotorA.Motor_Pwm,MotorB.Motor_Pwm);
			}else Set_PWM(0,0);
		}
    }
}

/**************************************************************************
Function: Get_Velocity_From_Encoder
Input   : none
Output  : none
函数功能：读取编码器和转换成速度
入口参数: 无 
返回  值：无
**************************************************************************/	 	
void Get_Velocity_From_Encoder(int Encoder1,int Encoder2)
{
	
	//Retrieves the original data of the encoder
	//获取编码器的原始数据
	float Encoder_A_pr,Encoder_B_pr; 
	OriginalEncoder.A=-Encoder1;	
	OriginalEncoder.B=-Encoder2;	
	Encoder_A_pr=OriginalEncoder.A; Encoder_B_pr=-OriginalEncoder.B;
	//编码器原始数据转换为车轮速度，单位m/s
	MotorA.Current_Encoder= Encoder_A_pr*Frequency*Perimeter/780.0f;  
	MotorB.Current_Encoder= Encoder_B_pr*Frequency*Perimeter/780.0f;   //1560=2*13*30=2（两路脉冲）*1（上升沿计数）*霍尔编码器13线*电机的减速比
	
}

//运动学逆解，由x和y的速度得到编码器的速度,Vx是m/s,Vz单位是度/s(角度制)
void Get_Target_Encoder(float Vx,float Vz)
{
	float amplitude=3.5f; //Wheel target speed limit //车轮目标速度限幅
	if(Vx<0) Vz=-Vz;
	else     Vz=Vz;
	//Inverse kinematics //运动学逆解
	 MotorA.Target_Encoder = Vx - Vz * Wheelspacing / 2.0f; //计算出左轮的目标速度
	 MotorB.Target_Encoder = Vx + Vz * Wheelspacing / 2.0f; //计算出右轮的目标速度
	//Wheel (motor) target speed limit //车轮(电机)目标速度限幅
//	 MotorA.Target_Encoder=target_limit_float( MotorA.Target_Encoder,-amplitude,amplitude); 
//	 MotorB.Target_Encoder=target_limit_float( MotorB.Target_Encoder,-amplitude,amplitude); 
}


/**************************************************************************
Function: Absolute value function
Input   : a：Number to be converted
Output  : unsigned int
函数功能：绝对值函数
入口参数：a：需要计算绝对值的数
返回  值：无符号整型
**************************************************************************/
int myabs(int a)
{
	int temp;
	if(a<0)  temp=-a;
	else temp=a;
	return temp;
}

int Turn_Off(void)
{
	u8 temp = 0;
//	if(Voltage>700&&EN==0)//电压高于7V且使能开关打开
//	{
//		temp = 1;
//	}
	return temp;			
}
/**************************************************************************
Function: PWM_Limit
Input   : IN;max;min
Output  : OUT
函数功能：限制PWM赋值
入口参数: IN：输入参数  max：限幅最大值  min：限幅最小值 
返回  值：限幅后的值
**************************************************************************/	 	
float PWM_Limit(float IN,float max,float min)
{
	float OUT = IN;
	if(OUT>max) OUT = max;
	if(OUT<min) OUT = min;
	return OUT;
}
/**************************************************************************
函数功能：增量PI控制器
入口参数：编码器测量值，目标速度
返回  值：电机PWM
根据增量式离散PID公式 
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)+Kd[e(k)-2e(k-1)+e(k-2)]
e(k)代表本次偏差 
e(k-1)代表上一次的偏差  以此类推 
pwm代表增量输出
在我们的速度控制闭环系统里面，只使用PI控制
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)
**************************************************************************/
int Incremental_PI_Left (float Encoder,float Target)
{ 	
	 float Bias;
	 Bias=Target-Encoder;                					//计算偏差
	 gPI_LeftPwm+=Velocity_KP*(Bias-gPI_LeftLastBias)+Velocity_KI*Bias;   	//增量式PI控制器
	if(Flag_Stop) gPI_LeftPwm=0;
	 if(gPI_LeftPwm>7800)gPI_LeftPwm=7800;
	 if(gPI_LeftPwm<-7800)gPI_LeftPwm=-7800;
	 gPI_LeftLastBias=Bias;                   					//保存上一次偏差 
	 return (int)gPI_LeftPwm;                         					//增量输出
}


int Incremental_PI_Right (float Encoder,float Target)
{ 	
	 float Bias;
	 Bias=Target-Encoder;                					//计算偏差
	 gPI_RightPwm+=Velocity_KP*(Bias-gPI_RightLastBias)+Velocity_KI*Bias;   	//增量式PI控制器
	if(Flag_Stop) gPI_RightPwm=0;
	 if(gPI_RightPwm>7800)gPI_RightPwm=7800;
	 if(gPI_RightPwm<-7800)gPI_RightPwm=-7800;
	 gPI_RightLastBias=Bias;                   					//保存上一次偏差 
	 return (int)gPI_RightPwm;                         					//增量输出
}
/**************************************************************************
Function: Processes the command sent by APP through usart 2
Input   : none
Output  : none
函数功能：对APP通过串口2发送过来的命令进行处理
入口参数：无
返回  值：无
**************************************************************************/
void Get_RC(void)
{
	u8 Flag_Move=1;
	 switch(Flag_Direction) //Handle direction control commands //处理方向控制命令
	 { 
			case 1:      Move_X=+RC_Velocity;  	 Move_Z=0;         break;
			case 2:      Move_X=+RC_Velocity;  	 Move_Z=-PI/2;   	 break;
			case 3:      Move_X=0;      				 Move_Z=-PI/2;   	 break;	 
			case 4:      Move_X=-RC_Velocity;  	 Move_Z=-PI/2;     break;		 
			case 5:      Move_X=-RC_Velocity;  	 Move_Z=0;         break;	 
			case 6:      Move_X=-RC_Velocity;  	 Move_Z=+PI/2;     break;	 
			case 7:      Move_X=0;     	 			 	 Move_Z=+PI/2;     break;
			case 8:      Move_X=+RC_Velocity; 	 Move_Z=+PI/2;     break; 
			default:     Move_X=0;               Move_Z=0;         break;
	 }
	 if     (Flag_Left ==1)  Move_Z= PI/2; //left rotation  //左自转
	 else if(Flag_Right==1)  Move_Z=-PI/2; //right rotation //右自转
//	}

	/* BT模式：超声波检测到前方近距障碍物时，禁止前进，允许左右转和后退 */
	if (BT_Obstacle_Near && Run_Mode == 0) {
		if (Move_X > 0.0f) {
			Move_X = 0.0f;
			Move_Z = 0.0f;
			/* 强制清空方向标志，防止APP持续发前进命令导致电机残余输出 */
			Flag_Direction = 0;
			Flag_Left = 0;
			Flag_Right = 0;
		}
	}

//	//Z-axis data conversion //Z轴数据转化
	if(Car_Mode==Akm_Car)
	{
		//Ackermann structure car is converted to the front wheel steering Angle system target value, and kinematics analysis is pearformed
		//阿克曼结构小车转换为前轮转向角度
		Move_Z=Move_Z*2/9; 
	}
	else if(Car_Mode==Diff_Car||Car_Mode==Tank_Car||Car_Mode==FourWheel_Car)
	{
//	  if(Move_X<0) Move_Z=-Move_Z; //The differential control principle series requires this treatment //差速控制原理系列需要此处理
		Move_Z=Move_Z*RC_Velocity/200;
	}		
	
	//Unit conversion, mm/s -> m/s
  //单位转换，mm/s -> m/s	
	Move_X=Move_X/1000;       Move_Y=Move_Y/1000;         Move_Z=Move_Z;

	/* Bluetooth straight-line heading hold using odometry */
	if (Odom_Enable && Run_Mode == 0) {
		static uint8_t bt_hh_locked = 0;
		static float bt_hh_target = 0.0f;
		float mz = (Move_Z >= 0.0f) ? Move_Z : -Move_Z;
		float mx = (Move_X >= 0.0f) ? Move_X : -Move_X;
		if (mz < 0.15f && mx > 0.05f) {
			if (!bt_hh_locked) {
				bt_hh_target = Odom_Theta;
				bt_hh_locked = 1;
			}
			float err = Odom_Theta - bt_hh_target;
			while (err > PI) err -= 2.0f * PI;
			while (err < -PI) err += 2.0f * PI;
			float corr = -1.0f * err;  /* weak correction */
			if (corr > 0.15f) corr = 0.15f;
			if (corr < -0.15f) corr = -0.15f;
			Move_Z += corr;
		} else {
			bt_hh_locked = 0;
		}
	}

	//Control target value is obtained and kinematics analysis is performed
	//得到控制目标值，进行运动学分析
	Get_Target_Encoder(Move_X,Move_Z);
}

/**************************************************************************
Function: Press the key to modify the car running state
Input   : none
Output  : none
函数功能：按键修改小车运行状态
入口参数：无
返回  值：无
**************************************************************************/
void Key(void)
{
	u8 tmp,tmp2;
	tmp=key_scan(200);//click_N_Double(50);
	if(tmp==USEKEY_single_click)
	{
		Flag_Stop=!Flag_Stop;
		printf("[KEY] Single click - Flag_Stop=%d\r\n", Flag_Stop);
	}		//单击控制小车的启停
	else if(tmp==USEKEY_double_click)
	{
		Run_Mode++;
		Run_Mode%=2;  /* 仅2种模式: 0=蓝牙遥控, 1=循迹 */
		printf("[KEY] Double click - Mode switched to %d (0=BT, 1=Line)\r\n", Run_Mode);
	}
	else if(tmp==USEKEY_triple_click)
	{
		/* Triple click: toggle inertial navigation (odometry) */
		Odom_Enable = !Odom_Enable;
		printf("[KEY] Triple click - Odom_Enable=%d\r\n", Odom_Enable);
		if (Odom_Enable) {
			odom_reset();
		}
	}
}
