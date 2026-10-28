/*
 * 模块名称: 通用定时器中断驱动 (timer)
 * 功能概述:
 *   - 配置TIM5定时器产生周期性中断
 *   - 可用于LVGL心跳、LED闪烁、GPS帧检测等周期性任务
 * 
 * 硬件资源:
 *   - TIM5: 32位通用定时器，挂载在APB1总线(84MHz)
 * 
 * 定时计算:
 *   Tout = ((arr+1) * (psc+1)) / Ft (us)
 *   其中Ft为定时器时钟频率(MHz)
 *   示例: arr=999, psc=83 -> Tout = 1000us = 1ms
 * 
 * 中断优先级:
 *   - 抢占优先级: 6 (>=5才能调用FreeRTOS API)
 *   - 子优先级: 0
 * 
 * 使用方法:
 *   1. 调用 TIM5_Init(999, 83) 初始化1ms定时中断
 *   2. 在 HAL_TIM_PeriodElapsedCallback() 中添加周期任务
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

extern TIM_HandleTypeDef TIM5_Handler;  // 定时器句柄

void TIM5_Init(u16 arr, u16 psc);       // 初始化TIM5定时器中断

#endif
