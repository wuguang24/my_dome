/**
 * @file    DHT11.h
 * @brief   DHT11温湿度传感器驱动头文件
 * @details 定义DHT11相关的宏定义和函数声明
 */

#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"                  // STM32F10x标准外设库头文件
#include "delay.h"                      // 延时函数头文件

/* ==================== 硬件引脚定义 ==================== */
// DHT11数据引脚：PB12
#define DHT11       GPIO_Pin_12
// DHT11端口：GPIOB
#define DHT11_PROT  GPIOB

/* ==================== 模式定义 ==================== */
// 输出模式
#define OUT 1
// 输入模式
#define INT 0

/* ==================== 电平控制宏 ==================== */
// DHT11拉低电平
#define DHT11_Low GPIO_ResetBits(DHT11_PROT,DHT11)
// DHT11拉高电平
#define DHT11_High GPIO_SetBits(DHT11_PROT,DHT11)

/* ==================== 函数声明 ==================== */
// DHT11初始化
void DHT11_Init(void);
// DHT11读取温湿度数据
void DHT11_Read_Data(u8 *humi, u8 *temp);

#endif
