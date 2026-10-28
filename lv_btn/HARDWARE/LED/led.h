/*
 * 模块名称: LED 驱动 (LED)
 * 功能概述:
 *   - 初始化板载LED引脚
 *   - LED0支持PWM调光，LED1支持开关控制
 * 硬件映射:
 *   - LED0(红灯): PA1 (TIM2_CH2 PWM, 低电平点亮, 0..500)
 *   - LED1(绿灯): PA2 (推挽输出, 低电平点亮)
 */
#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F407ZGT6开发板
//LED驱动代码	   
//版本：V1.0									  
////////////////////////////////////////////////////////////////////////////////// 	

// 板载 LED 宏定义
#define LED0 PAout(1)   	// LED0 - 红灯
#define LED1 PAout(2)   	// LED1 - 绿灯

void LED_Init(void);              // LED 初始化: PA1(PWM)/PA2(GPIO)
void LED0_SetPWM(u16 duty);       // LED0(红灯)PWM调光: 0(灭)..500(最亮), 低电平有效
void LED0_Set(u8 on);             // LED0(红灯)快捷开关: on=1 点亮 / on=0 熄灭
void LED1_Set(u8 on);             // LED1(绿灯)控制: on=1 点亮 / on=0 熄灭
#endif
