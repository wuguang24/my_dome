/**
 * @file    Key.h
 * @brief   按键驱动头文件
 * @details 定义按键相关的宏定义和函数声明
 */

#ifndef	__KEY_H
#define __KEY_H

#include "stm32f10x.h"                  // STM32F10x标准外设库头文件

/* ==================== 硬件引脚定义 ==================== */
// KEY1引脚：PB8
#define KEY1_GPIO_PIN	GPIO_Pin_8
// KEY2引脚：PB7
#define KEY2_GPIO_PIN	GPIO_Pin_7
// KEY3引脚：PB6
#define KEY3_GPIO_PIN	GPIO_Pin_6
// KEY4引脚：PB5
#define KEY4_GPIO_PIN	GPIO_Pin_5

// 按键端口：GPIOB
#define KEY_PROT	GPIOB
 
/* ==================== 按键读取宏 ==================== */
// 读取KEY1状态：1=未按下，0=按下
#define KEY1	GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO_PIN)
// 读取KEY2状态：1=未按下，0=按下
#define KEY2	GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO_PIN)
// 读取KEY3状态：1=未按下，0=按下
#define KEY3	GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO_PIN)
// 读取KEY4状态：1=未按下，0=按下
#define KEY4	GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO_PIN)

/* ==================== 按键时间参数定义 ==================== */
// 消抖时间：约20ms（扫描周期2ms * 10次）
#define KEY_DELAY_TIME				10
// 长按触发时间：约4s（扫描周期2ms * 2000次）
#define KEY_LONG_TIME				2000
// KEY1长按触发时间：约1.6s（扫描周期2ms * 800次）
#define KEY1_LONG_TIME				800

// 连击触发开始时间：约1s（扫描周期2ms * 500次）
#define KEY_Continue_TIME			500
// 连击触发间隔：约30ms（扫描周期2ms * 15次）
#define KEY_Continue_Trigger_TIME	15

/* ==================== 全局变量声明 ==================== */
// 当前按键键值（外部可访问）
extern u8 KeyNum;

/* ==================== 函数声明 ==================== */
// 按键初始化
void Key_Init(void);
// 按键扫描（需周期性调用）
void Key_scan(void);

#endif
