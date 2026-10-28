#ifndef __FAN_H
#define	__FAN_H
#include "stm32f10x.h"
#include "delay.h"

/*****************风扇模块头文件******************
 * 文件			: 5V风扇模块h文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.21
 * MCU			: STM32F103C8T6
 * 接口			: 通用IO口									
 * 作者			: 野火 
 * 功能			: 控制风扇的开关
**********************BEGIN***********************/

/***************用户自定义区域****************/
// 风扇模块 GPIO定义

#define	FAN_CLK					RCC_APB2Periph_GPIOB  // 风扇GPIO时钟

#define FAN_GPIO_PIN 			GPIO_Pin_0           // 风扇GPIO引脚

#define FAN_GPIO_PROT 			GPIOB                // 风扇GPIO端口

#define FAN_ON 		GPIO_SetBits(FAN_GPIO_PROT, FAN_GPIO_PIN)   // 打开风扇
#define FAN_OFF 	GPIO_ResetBits(FAN_GPIO_PROT, FAN_GPIO_PIN) // 关闭风扇

/*********************END**********************/

// 风扇初始化函数
void FAN_Init(void);

#endif



