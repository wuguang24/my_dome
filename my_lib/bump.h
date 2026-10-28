#ifndef __BUMP_H
#define	__BUMP_H
#include "stm32f10x.h"
#include "delay.h"

/*****************水泵模块头文件******************
 * 文件			: 5V水泵模块h文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.22
 * MCU			: STM32F103C8T6
 * 接口			: 通用IO口									
 * 作者			: 野火 
 * 功能			: 控制水泵的开关
**********************BEGIN***********************/

/***************用户自定义区域****************/
// 水泵模块 GPIO定义

#define	BUMP_CLK					RCC_APB2Periph_GPIOB  // 水泵GPIO时钟

#define BUMP_GPIO_PIN 			GPIO_Pin_1           // 水泵GPIO引脚

#define BUMP_GPIO_PROT 			GPIOB                // 水泵GPIO端口

#define BUMP_ON 		GPIO_SetBits(BUMP_GPIO_PROT, BUMP_GPIO_PIN)   // 打开水泵
#define BUMP_OFF 	GPIO_ResetBits(BUMP_GPIO_PROT, BUMP_GPIO_PIN) // 关闭水泵

/*********************END**********************/

// 水泵初始化函数
void BUMP_Init(void);

#endif



