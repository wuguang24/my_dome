#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"

// USART1 接口（对应 USART1.c）
// 本工程主要用于 JW01 CO2 模块接收数据。
// 接收方式：RXNE 中断累积到内部缓冲区；上层轮询读取后调用 Clear 清空。
void USART1_Init(uint32_t wBaudRate);
void USART1_Send_Data(uint8_t *pbData, uint8_t bDataLen);

// 清空接收缓冲区（长度归零并 memset 清零）
void USART1_Data_Clear(void);
// 获取接收缓冲区指针（返回内部静态数组首地址）
uint8_t *USART1_Data_Get(void);
// 获取当前接收长度（字节数）
uint16_t USART1_Data_Len_Get(void);

#endif
