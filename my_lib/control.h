#ifndef	__CONTROL_H
#define	__CONTROL_H
#include "stm32f10x.h"                  // Device header

// 蜂鸣器定义
#define BEEP  PAout(8)  // 蜂鸣器控制引脚（PA8）

// 函数声明

// 蜂鸣器初始化函数
void beep_init(void);
// LED初始化函数
void led_Init(void);

#endif
