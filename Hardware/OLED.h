/**
 * @file    OLED.h
 * @brief   OLED显示屏驱动头文件
 * @details 定义OLED相关的宏定义和函数声明
 */

#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"          // STM32F10x标准外设库头文件

/* ==================== 硬件引脚定义 ==================== */
// OLED SCL引脚：PB1
#define OLED_SCL			GPIO_Pin_1
// OLED SCL端口：GPIOB
#define OLED_PROT  			GPIOB

// OLED SDA引脚：PB0
#define OLED_SDA			GPIO_Pin_0
// OLED SDA端口：GPIOB
#define OLED_PROT2  		GPIOB

/* ==================== I2C电平控制宏 ==================== */
// SCL电平控制：x=1拉高，x=0拉低
#define OLED_W_SCL(x)		GPIO_WriteBit(OLED_PROT, OLED_SCL, (BitAction)(x))
// SDA电平控制：x=1拉高，x=0拉低
#define OLED_W_SDA(x)		GPIO_WriteBit(OLED_PROT2, OLED_SDA, (BitAction)(x))

/* ==================== 函数声明 ==================== */
// OLED初始化
void OLED_Init(void);
// OLED清屏
void OLED_Clear(void);
// OLED显示一个字符
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
// OLED显示字符串
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
// OLED显示数字（十进制，正数）
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
// OLED显示数字（十进制，带符号数）
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
// OLED显示数字（十六进制）
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
// OLED显示数字（二进制）
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
// OLED显示一个汉字
void OLED_ShowChinese(uint8_t Line, uint8_t Column, uint8_t num);

#endif
