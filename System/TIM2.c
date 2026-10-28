#include "tim2.h"                  // Device header
uint32_t delay20ms;
uint8_t keyDelay_50ms = 150;
static volatile uint32_t tim2_ms = 0;

uint32_t TIM2_GetMs(void)
{
	return tim2_ms;
}

void Timer2_Init(u16 Prescaler, u16 Period)	//0-65535
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = Period;
	TIM_TimeBaseInitStructure.TIM_Prescaler = Prescaler;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)	//2ms定时中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET ) 
	{
		tim2_ms += 2;
		Key_scan();	//按键扫描函数
		
		if (keyDelay_50ms)
		{
			keyDelay_50ms--;
			KeyNum = 0;	
		}
		
		// HC-SR04 模块当前工程未使用：不再在 2ms 中断里维护其延时状态机
		
		//机智云APP控制LED的延迟
		delay20ms++;
		if(delay20ms >= 10)
		{
			delay20ms = 0;
			LED_APP_Command();	
		}
		
		// 注意：蜂鸣器由主循环的“缺水保护”统一控制。
		// 这里不再做“距离阈值报警”，避免与缺水蜂鸣器逻辑冲突导致一直响。
		
		// Gizwits 模块已移除：不再处理配网/模式切换
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);				
	}	
}

