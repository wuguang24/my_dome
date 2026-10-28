/**
 * @file    jsq.h
 * @brief   加湿器（JSQ）驱动头文件
 * @details 定义加湿器相关的宏定义和函数声明
 */

#ifndef __JSQ_H
#define	__JSQ_H

#include "stm32f10x.h"              // STM32F10x标准外设库头文件

/* ==================== 加湿器控制引脚定义 ==================== */
// 加湿器控制引脚时钟：GPIOA
#define	JSQ_CLK			RCC_APB2Periph_GPIOA
// 加湿器控制引脚：PA1
#define JSQ_GPIO_PIN 	GPIO_Pin_1
// 加湿器控制端口：GPIOA
#define JSQ_GPIO_PORT 	GPIOA

/* ==================== 加湿器按键引脚定义 ==================== */
// 加湿器按键引脚时钟：GPIOB
#define JSQ_KEY_CLK     RCC_APB2Periph_GPIOB
// 加湿器按键端口：GPIOB
#define JSQ_KEY_PORT    GPIOB
// 加湿器按键引脚：PB9（预留，当前使用Key.c的按键扫描）
#define JSQ_KEY_PIN     GPIO_Pin_9

/* ==================== 加湿器开关控制宏 ==================== */
// 打开加湿器（PA1输出高电平）
#define JSQ_ON 	  GPIO_SetBits(JSQ_GPIO_PORT, JSQ_GPIO_PIN)
// 关闭加湿器（PA1输出低电平）
#define JSQ_OFF 	GPIO_ResetBits(JSQ_GPIO_PORT, JSQ_GPIO_PIN)

/* ==================== 函数声明 ==================== */
// 加湿器初始化
void JSQ_Init(void);

#endif
