/*
 * 模块名称: 蜂鸣器驱动 (BEEP)
 * 功能概述:
 *   - 初始化 PG13 为推挽输出
 *   - 提供蜂鸣器开关/翻转/节奏鸣叫接口
 * 硬件映射:
 *   - BEEP: PG13 (高电平=鸣叫)
 */
#ifndef _BEEP_H
#define _BEEP_H
#include "sys.h"

// 蜂鸣器引脚定义 (PG13)
#define BEEP_GPIO_PORT     GPIOG                    // 端口
#define BEEP_GPIO_PIN      GPIO_PIN_13              // 引脚
#define BEEP_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOG_CLK_ENABLE() // 时钟使能

// 蜂鸣器操作宏定义
#define BEEP_ON()     HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN, GPIO_PIN_SET)   // 置高=鸣叫
#define BEEP_OFF()    HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN, GPIO_PIN_RESET) // 置低=静音
#define BEEP_TOGGLE() HAL_GPIO_TogglePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN)                // 翻转

// 函数声明
void BEEP_Init(void);                                                     // 初始化 PG13 输出
void BEEP_Set(uint8_t state);                                             // 设置 1=鸣叫 / 0=静音
void BEEP_Beep(uint16_t times, uint16_t on_time_ms, uint16_t off_time_ms);// 节奏鸣叫(FreeRTOS 延时)

#endif

