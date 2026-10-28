/**
 * @file    Buzzer.c
 * @brief   蜂鸣器驱动程序
 * @details 实现蜂鸣器的初始化、开启、关闭功能
 *          硬件连接：PA5引脚，高电平触发
 */

#include "buzzer.h"

/* ==================== 全局变量定义 ==================== */
// 蜂鸣器状态标志：1=开启，0=关闭
u8 Buzzer_Flag;

/**
  * @brief  蜂鸣器初始化函数
  * @param  无
  * @retval 无
  * @note   配置PA5为推挽输出模式，默认关闭蜂鸣器
  */
void Buzzer_Init(void)
{
	// 使能蜂鸣器GPIO时钟（GPIOA）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// 配置GPIO结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	// 推挽输出模式：本项目硬件为高电平触发
	// 输出高电平=蜂鸣器响，输出低电平=蜂鸣器停
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	// 蜂鸣器引脚：PA5
	GPIO_InitStructure.GPIO_Pin = Buzzer;
	// 输出速度：50MHz
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// 初始化GPIO
	GPIO_Init(Buzzer_PROT, &GPIO_InitStructure);
	
	// 默认关闭蜂鸣器（拉低电平）
	GPIO_ResetBits(Buzzer_PROT, Buzzer);
}

/**
  * @brief  蜂鸣器开启函数
  * @param  无
  * @retval 无
  * @note   置位软件标志，并拉高GPIO电平
  */
void Buzzer_ON(void)
{
	// 置位软件标志，表示蜂鸣器已开启
	Buzzer_Flag = 1;
	// 拉高GPIO电平，蜂鸣器开始鸣响
	GPIO_SetBits(Buzzer_PROT, Buzzer);
}

/**
  * @brief  蜂鸣器关闭函数
  * @param  无
  * @retval 无
  * @note   清零软件标志，并拉低GPIO电平
  */
void Buzzer_OFF(void)
{
	// 清零软件标志，表示蜂鸣器已关闭
	Buzzer_Flag = 0;
	// 拉低GPIO电平，蜂鸣器停止鸣响
	GPIO_ResetBits(Buzzer_PROT, Buzzer);
}
