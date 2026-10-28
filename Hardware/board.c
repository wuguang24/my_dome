#include "ti_msp_dl_config.h"
#include "board.h"

volatile unsigned long tick_ms;
volatile uint32_t start_time;

/* 舵机使用TIMA0硬件PWM，PA8引脚，50Hz频率 */
static volatile uint16_t g_servo_cc_value = 1500;  // 当前PWM占空比值(500-2500)
static volatile uint8_t g_servo_enable = 0;

#ifndef SERVO_SAFE_MIN_DEG
#define SERVO_SAFE_MIN_DEG (0U)
#endif

#ifndef SERVO_SAFE_MAX_DEG
#define SERVO_SAFE_MAX_DEG (180U)
#endif





//printf函数重定义
int fputc(int ch, FILE *stream)
{
	//当串口0忙的时候等待，不忙的时候再发送传进来的字符
	while( DL_UART_Main_isBusy(UART_0_INST) == true );

	DL_UART_Main_transmitDataBlocking(UART_0_INST, ch);

	return ch;
}

int fputs(const char* restrict s,FILE* restrict stream)
{
   uint16_t i,len;
   len = strlen(s);
   for(i=0;i<len;i++)
   {
       DL_UART_Main_transmitDataBlocking(UART_0_INST,s[i]);
   }
   return len;
}

int puts(const char *_ptr)
{
    int count = fputs(_ptr,stdout);
    count += fputs("\n",stdout);
    return count;
}


//返回SysTick计数值
uint32_t Systick_getTick(void)
{
	return (SysTick->VAL);
}


//ms阻塞延迟
void delay_ms(uint32_t ms)
{
	//超出能满足的最大延迟
	//if( ms > SysTickMAX_COUNT/(SysTickFre/1000) ) ms = SysTickMAX_COUNT/(SysTickFre/1000);
	for(int i=0;i<1000;i++)
	{
		delay_us(ms);
	}
}


void delay_us(uint32_t us)
{
	if( us > (SysTick->LOAD+1U)/(SysTickFre/1000000) ) us = (SysTick->LOAD+1U)/(SysTickFre/1000000);
	
	us = us*(SysTickFre/1000000); //单位转换
	
	//用于保存已走过的时间
	uint32_t runningtime = 0;
	
	//获得当前时刻的计数值
	uint32_t InserTick = Systick_getTick();
	
	//用于刷新实时时间
	uint32_t tick = 0;
	
	uint8_t countflag = 0;

	// SysTick实际周期（LOAD+1），用于正确计算溢出
	uint32_t systick_period = (uint32_t)(SysTick->LOAD) + 1U;

	//等待延迟
	while(1)
	{
		tick = Systick_getTick();//刷新当前时刻计数值
		
		if( tick > InserTick ) countflag = 1;//出现溢出轮询,则切换走时的计算方式
		
		if( countflag ) runningtime = InserTick + systick_period - tick;
		else runningtime = InserTick - tick;
		
		if( runningtime>=us ) break;
	}

}


void delay_1us(unsigned long __us){ delay_us(__us); }
void delay_1ms(unsigned long ms){ delay_ms(ms); }

void SysTick_Handler(void)
{
	tick_ms++;
}

void Servo_SetAngleDeg(uint8_t angle_deg)
{
	if (angle_deg < SERVO_SAFE_MIN_DEG) {
		angle_deg = SERVO_SAFE_MIN_DEG;
	}
	if (angle_deg > SERVO_SAFE_MAX_DEG) {
		angle_deg = SERVO_SAFE_MAX_DEG;
	}
	/* Map 0..180 deg -> 500..2500 us pulse width (50Hz, 1MHz tick) */
	g_servo_cc_value = 500U + ((uint32_t)angle_deg * 2000U) / 180U;
	if (g_servo_enable) {
		DL_TimerA_setCaptureCompareValue(PWM_SERVO_INST, g_servo_cc_value, GPIO_PWM_SERVO_C0_IDX);
	}
}

void Servo_Enable(uint8_t en)
{
	g_servo_enable = (en != 0U) ? 1U : 0U;
	if (!g_servo_enable) {
		/* 禁用PWM输出，设置占空比为0 */
		DL_TimerA_setCaptureCompareValue(PWM_SERVO_INST, 0, GPIO_PWM_SERVO_C0_IDX);
	} else {
		/* 设置当前位置 */
		DL_TimerA_setCaptureCompareValue(PWM_SERVO_INST, g_servo_cc_value, GPIO_PWM_SERVO_C0_IDX);
	}
}

void Servo_Task20ms(void)
{
	/* 硬件PWM不需要软件维护，此函数保留用于兼容性 */
}

void Servo_TimerInit(void)
{
	/* TIMA0硬件PWM已在ti_msp_dl_config.c中初始化
	 * 但默认prescale=0导致80MHz/20000=4kHz，舵机需要50Hz
	 * 需要重新配置时钟：prescale=79, 80MHz/80=1MHz, 1MHz/20000=50Hz
	 */
	DL_TimerA_stopCounter(PWM_SERVO_INST);

	DL_TimerA_ClockConfig servoClkCfg = {
		.clockSel    = DL_TIMER_CLOCK_BUSCLK,
		.divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
		.prescale    = 79U,
	};
	DL_TimerA_setClockConfig(PWM_SERVO_INST, &servoClkCfg);

	DL_TimerA_PWMConfig servoPwmCfg = {
		.pwmMode        = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
		.period         = 20000,
		.isTimerWithFourCC = true,
		.startTimer     = DL_TIMER_START,
	};
	DL_TimerA_initPWMMode(PWM_SERVO_INST, &servoPwmCfg);

	DL_TimerA_setCaptureCompareValue(PWM_SERVO_INST, g_servo_cc_value, GPIO_PWM_SERVO_C0_IDX);
	DL_TimerA_setCaptureCompareValue(PWM_SERVO_INST, 0, GPIO_PWM_SERVO_C1_IDX);

	DL_TimerA_enableClock(PWM_SERVO_INST);
}
