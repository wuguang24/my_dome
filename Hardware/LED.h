/**
 * @file    LED.h
 * @brief   LED灯驱动头文件
 * @details 定义LED相关的宏定义和函数声明
 */

#ifndef	_LED_H_
#define	_LED_H_

#include "stm32f10x.h"                  // STM32F10x标准外设库头文件
#include "key.h"                        // 按键驱动头文件
#include "sensormodules.h"              // 传感器模块数据结构头文件
#include "tim2.h"                       // TIM2定时器头文件

/* ==================== 硬件引脚定义 ==================== */
// LED引脚：PB13（TIM1_CH1N）
#define LED				GPIO_Pin_13
// LED端口：GPIOB
#define LED_PROT		GPIOB

/* ==================== 亮度参数定义 ==================== */
// LED亮度数值上限：100%
#define LED_PWM_UPPER_LIMIT			100

/* ==================== 全局变量声明 ==================== */
// LED亮度占空比（0-100）
extern uint8_t ledDutyRatio;

/* ==================== 函数声明 ==================== */
// LED初始化
void LED_Init(void);
// LED PWM按键控制
void LED_PWM_KEY(void);
// LED APP命令处理
void LED_APP_Command(void);

#endif
