#ifndef __BUZZ_H
#define __BUZZ_H

#include "stm32f10x.h"

// 蜂鸣器引脚定义
#define BUZZ_PIN    GPIO_Pin_14   // 引脚
#define BUZZ_PORT   GPIOB

// 函数声明

// 蜂鸣器初始化函数
void Buzz_Init(void);
// 关闭蜂鸣器函数
void Buzz_Off(void);
// 开启蜂鸣器函数
void Buzz_ON(void);

#endif

