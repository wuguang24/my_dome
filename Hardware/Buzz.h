#ifndef __BUZZ_H
#define __BUZZ_H

#include "stm32f10x.h"

// 蜂鸣器接口（对应 Buzz.c）
// 说明：本工程蜂鸣器为低电平触发：GPIO 输出 0 时响，输出 1 时停。
// 引脚以 BUZZ_PORT / BUZZ_PIN 为准。
// 蜂鸣器引脚定义
#define BUZZ_PIN    GPIO_Pin_4   // PA12引脚
#define BUZZ_PORT   GPIOA

// 函数声明
// 룝츤포多신땍屢
// 변鑒�츠

void Buzz_Init(void);
void Buzz_Off(void);
void Buzz_ON(void);

#endif

