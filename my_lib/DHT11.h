#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

#define DHT11_PIN  GPIO_Pin_8  // DHT11传感器引脚

// 函数返回值
#define SUCCESS    0  // 成功
#define ERROR      1  // 错误

// 函数声明
// DHT11传感器初始化函数
void DHT11_Init(void);
// 读取DHT11温湿度数据
// 参数：temp - 温度值指针，humi - 湿度值指针
// 返回值：SUCCESS - 读取成功，ERROR - 读取失败
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#endif
