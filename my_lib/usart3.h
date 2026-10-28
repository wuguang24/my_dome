#ifndef __usart3_H                          // 防止头文件重复包含
#define __usart3_H	                       // 头文件宏定义开始
#include "stm32f10x.h"                    // 引入STM32F10x外设库头文件（GPIO/RCC等定义）
#include "stm32f10x_usart.h"              // 引入USART外设库头文件（USART函数声明）

#define USART3_RXBUFF_SIZE   512            // USART3接收缓冲区大小（字节）

extern unsigned int Rx3Counter;             // USART3接收计数（外部声明，其他文件可访问）
extern char Usart3RecBuf[USART3_RXBUFF_SIZE]; // USART3接收缓冲数组（外部声明，其他文件可访问）

void USART3_Init(u32 baud);                 // USART3初始化函数声明（baud为波特率）
void USART3_Sned_Char(u8 temp);             // USART3发送单字节函数声明（轮询发送）
void Uart3_SendStr(char*SendBuf);           // USART3发送字符串函数声明（以'\0'结束）
void uart3_send(unsigned char *bufs,unsigned char len); // USART3发送缓冲区函数声明（len=0xFF表示按字符串发送）

#endif                                     // 头文件宏定义结束
