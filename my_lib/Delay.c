#include "stm32f10x.h"                   // 引入STM32F10x外设库头文件（包含SysTick寄存器定义）

/**                                 // 函数说明注释开始
  * @brief  微秒级延时                 // 功能：提供微秒级软件延时
  * @param  xus 延时时长，范围：0~233015 // 入参：需要延时的微秒数（受SysTick计数范围限制）
  * @retval 无                         // 返回值：无
  */                                 // 函数说明注释结束
void Delay_us(uint32_t xus)           // 微秒延时函数定义
{                                    // 函数体开始
	SysTick->LOAD = 72 * xus;				// 设置SysTick重装载值：72MHz下每微秒72个时钟
	SysTick->VAL = 0x00;					// 清空SysTick当前计数值
	SysTick->CTRL = 0x00000005;				// 选择HCLK为时钟源并使能计数器（不使能中断）
	while(!(SysTick->CTRL & 0x00010000));	// 等待COUNTFLAG置位，表示计数到0
	SysTick->CTRL = 0x00000004;				// 关闭计数器（保持时钟源位）
}                                    // 函数体结束

/**                                 // 函数说明注释开始
  * @brief  毫秒级延时                 // 功能：提供毫秒级软件延时
  * @param  xms 延时时长，范围：0~4294967295 // 入参：需要延时的毫秒数（32位计数）
  * @retval 无                         // 返回值：无
  */                                 // 函数说明注释结束
void Delay_ms(uint32_t xms)           // 毫秒延时函数定义
{                                    // 函数体开始
	while(xms--)                        // 循环xms次，每次延时1ms
	{                                  // while循环体开始
		Delay_us(1000);                  // 调用微秒延时实现1ms延时
	}                                  // while循环体结束
}                                    // 函数体结束
                                     
/**                                 // 函数说明注释开始
  * @brief  秒级延时                   // 功能：提供秒级软件延时
  * @param  xs 延时时长，范围：0~4294967295 // 入参：需要延时的秒数（32位计数）
  * @retval 无                         // 返回值：无
  */                                 // 函数说明注释结束
void Delay_s(uint32_t xs)            // 秒延时函数定义
{                                    // 函数体开始
	while(xs--)                         // 循环xs次，每次延时1秒
	{                                  // while循环体开始
		Delay_ms(1000);                  // 调用毫秒延时实现1秒延时
	}                                  // while循环体结束
}                                    // 函数体结束
