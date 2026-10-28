#ifndef __LED_MODULE_H
#define	__LED_MODULE_H
#include "stm32f10x.h"
#include "delay.h"

// LED模块引脚定义

#define	LED_MODULE_CLK					RCC_APB2Periph_GPIOA  // LED模块GPIO时钟

#define LED_MODULE_GPIO_PIN 			GPIO_Pin_3           // LED模块GPIO引脚

#define LED_MODULE_GPIO_PORT 			GPIOA                // LED模块GPIO端口

#define LED_MODULE_ON 					GPIO_SetBits(LED_MODULE_GPIO_PORT, LED_MODULE_GPIO_PIN)   // 打开LED模块
#define LED_MODULE_OFF 					GPIO_ResetBits(LED_MODULE_GPIO_PORT, LED_MODULE_GPIO_PIN) // 关闭LED模块

// LED模块初始化函数
void LED_MODULE_Init(void);

#endif



