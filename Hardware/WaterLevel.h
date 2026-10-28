/**
 * @file    WaterLevel.h
 * @brief   水位传感器驱动头文件
 * @details 定义水位传感器相关的宏定义和函数声明
 * 
 * 水位检测工作原理：
 * 通过导电检测液位，有液体时输出低电平，没有液体时输出高电平
 */

#ifndef __WATERLEVEL_H
#define __WATERLEVEL_H

#include "stm32f10x.h"              // STM32F10x标准外设库头文件

/* ==================== 硬件引脚定义 ==================== */
// 水位传感器时钟：GPIOA
#define WATERLEVEL_CLK      RCC_APB2Periph_GPIOA
// 水位传感器端口：GPIOA
#define WATERLEVEL_PORT     GPIOA
// 水位传感器引脚：PA4
#define WATERLEVEL_PIN      GPIO_Pin_4

/* ==================== 函数声明 ==================== */
// 水位传感器初始化
void WaterLevel_Init(void);
// 水位检测（返回1=有水，0=缺水）
uint8_t WaterLevel_IsWaterDetected(void);

#endif
