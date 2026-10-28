#ifndef __usart1_H                          // 防止头文件重复包含
#define __usart1_H	                       // 头文件宏定义开始
#include "stm32f10x.h"                    // 引入STM32F10x外设库头文件（USART/GPIO等定义）

#define USART1_RXBUFF_SIZE   90            // USART1接收缓冲区大小（字节）

extern unsigned int RxCounter;             // USART1接收计数（外部声明，其他文件可访问）
extern char Usart1RecBuf[USART1_RXBUFF_SIZE]; // USART1接收缓冲数组（外部声明，其他文件可访问）

void uart1_Init(u32 bound);                // USART1初始化函数声明（bound为波特率）
void uart1_SendStr(char*SendBuf);          // USART1发送字符串函数声明（以'\0'结束）
void uart1_send(unsigned char *bufs,unsigned char len); // USART1发送缓冲区函数声明（len=0xFF表示按字符串发送）
		 			                       // 预留空行占位（不影响功能）

#endif                                     // 头文件宏定义结束
