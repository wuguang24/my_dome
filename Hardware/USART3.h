#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"

// USART3 调试输出接口（对应 USART3.c）
// 说明：当前仅初始化 Tx，并提供阻塞式发送函数。

// 串口打印控制宏：
// - 定义 ENABLE_UART_PRINT：启用 USART3 调试输出（占用 PB10=USART3_TX）
// - 不定义 ENABLE_UART_PRINT：USART3 接口为空实现（不初始化 GPIO/外设，从而可复用 PB10）
#ifdef ENABLE_UART_PRINT
void USART3_Init(uint32_t wBaudRate);
void USART3_Send_Data(uint8_t *pbData, uint8_t bDataLen);
#else
// 未使能打印时，将调用替换为空操作，避免与 USART3.c 中的真实实现产生重复定义。
#define USART3_Init(wBaudRate)            ((void)(wBaudRate))
#define USART3_Send_Data(pbData, bDataLen) ((void)(pbData), (void)(bDataLen))
#endif

#endif
