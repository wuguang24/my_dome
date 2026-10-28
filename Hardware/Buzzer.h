/**
 * @file    Buzzer.h
 * @brief   蜂鸣器驱动头文件
 * @details 定义蜂鸣器相关的宏定义和函数声明
 */

#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"                  // STM32F10x标准外设库头文件

/* ==================== 硬件引脚定义 ==================== */
// 蜂鸣器引脚：PA5
#define Buzzer			GPIO_Pin_5
// 蜂鸣器端口：GPIOA
#define Buzzer_PROT		GPIOA

/* ==================== 全局变量声明 ==================== */
// 蜂鸣器状态标志（外部可访问）
extern u8 Buzzer_Flag;

/* ==================== 函数声明 ==================== */
// 蜂鸣器初始化
void Buzzer_Init(void);
// 蜂鸣器开启
void Buzzer_ON(void);
// 蜂鸣器关闭
void Buzzer_OFF(void);

#endif
