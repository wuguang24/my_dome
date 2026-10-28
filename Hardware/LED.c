/**
 * @file    LED.c
 * @brief   LED灯驱动程序
 * @details 实现LED的初始化、PWM亮度控制功能
 *          硬件连接：PB13引脚，TIM1_CH1N PWM输出
 */

#include "led.h"

/* ==================== 全局变量定义 ==================== */
// LED亮度占空比：0-100，表示亮度百分比
uint8_t ledDutyRatio = 0;

/**
  * @brief  LED初始化函数
  * @param  无
  * @retval 无
  * @note   配置PB13为复用推挽输出，用于TIM1 PWM输出
  */
void LED_Init(void)
{
	// 开启GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 配置GPIO结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	// 复用推挽输出模式：用于PWM输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	// LED引脚：PB13（TIM1_CH1N）
	GPIO_InitStructure.GPIO_Pin = LED;
	// 输出速度：50MHz
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// 初始化GPIO
	GPIO_Init(LED_PROT, &GPIO_InitStructure);
	
	// 默认关闭LED（拉低电平）
	GPIO_ResetBits(LED_PROT, LED);
}

/**
  * @brief  LED PWM按键控制函数
  * @param  无
  * @retval 无
  * @note   根据按键值调节LED亮度
  *         KEY2：亮度增加
  *         KEY3：亮度减少
  *         KEY4：关闭LED
  */
void LED_PWM_KEY(void)
{
	// KEY2：亮度增加
	if (KeyNum == 2)
	{
		KeyNum = 0;  // 清除按键标志
		ledDutyRatio++;  // 亮度+1
		if (ledDutyRatio >= LED_PWM_UPPER_LIMIT)
		{
			ledDutyRatio = 0;  // 超过上限则回到0
		}
	}
	// KEY3：亮度减少
	if (KeyNum == 3)
	{
		KeyNum = 0;  // 清除按键标志
		ledDutyRatio--;  // 亮度-1
		if (ledDutyRatio >= LED_PWM_UPPER_LIMIT)
		{
			ledDutyRatio = LED_PWM_UPPER_LIMIT;  // 下溢则设为上限
		}
	}
	// KEY4：关闭LED
	if (KeyNum == 4)
	{
		KeyNum = 0;  // 清除按键标志
		ledDutyRatio = 0;  // 亮度设为0
	}
	// 设置PWM占空比，更新LED亮度
	TIM_SetCompare1(TIM1, ledDutyRatio);
}

/**
  * @brief  LED APP命令处理函数
  * @param  无
  * @retval 无
  * @note   当前工程已移除Gizwits/上位机控制逻辑：LED亮度由main.c决定
  */
void LED_APP_Command(void)
{
	// 当前工程已移除 Gizwits/上位机控制逻辑：LED 亮度由 main.c 决定
}
