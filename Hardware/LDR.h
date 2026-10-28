/**
 * @file    LDR.h
 * @brief   光敏电阻（LDR）驱动头文件
 * @details 定义光敏电阻相关的宏定义和函数声明
 */

#ifndef _LDR_H_
#define _LDR_H_

#include "stm32f10x.h"                  // STM32F10x标准外设库头文件
#include "adcx.h"                       // ADC驱动头文件
#include "Delay.h"                      // 延时函数头文件
#include <math.h>                       // 数学库头文件（用于pow函数）

/* ==================== 采样参数定义 ==================== */
// 多次采样次数，用于取平均值提高稳定性
#define LDR_READ_TIMES	10

/* ==================== 硬件引脚定义 ==================== */
// LDR引脚：PA0（ADC通道0）
#define LDR				GPIO_Pin_0
// LDR端口：GPIOA
#define LDR_PROT		GPIOA

/* ==================== 函数声明 ==================== */
// LDR初始化
void LDR_Init(void);
// LDR多次采样取平均值
uint16_t LDR_Average_Data(void);
// LDR光照强度计算
void LDR_LuxData(uint16_t *Lux);

#endif
