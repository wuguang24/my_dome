#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

// DHT11 数据脚连接
// 说明：DHT11 为单总线，数据线需要上拉（内部上拉/外部上拉均可）。
// 本工程默认使用 PA8。
#define DHT11_PIN  GPIO_Pin_8
#define DHT11_PORT GPIOA

// 函数返回值
// SUCCESS：读取并校验通过
// ERROR  ：超时/校验失败等
#define SUCCESS    0
#define ERROR      1

// 函数声明
// DHT11_Init：初始化 GPIO 与上电稳定等待
// DHT11_Read_Data：读取一次温湿度（只返回整数部分）
void DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#endif
