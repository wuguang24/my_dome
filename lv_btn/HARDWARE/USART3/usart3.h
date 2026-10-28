/*
 * 模块名称: USART3串口驱动 (usart3)
 * 功能概述:
 *   - 配置USART3用于雷达模块通信
 *   - 支持中断接收，缓冲区管理
 * 
 * 硬件连接:
 *   - USART3_TX: PB10
 *   - USART3_RX: PB11
 *   - 波特率: 115200bps
 * 
 * 通信对象:
 *   - 雷达模块(人体检测传感器 R60ABD1)
 *   - 协议: 私有二进制协议
 * 
 * 缓冲区说明:
 *   - USART3_RX_BUF[]: 接收缓冲区，最大1024字节
 *   - USART3_RX_STA: 接收状态标记
 *     - bit15: 接收完成标志
 *     - bit14: 接收到0x0d(回车符)
 *     - bit13~0: 接收到的有效字节数目
 * 
 * 使用方法:
 *   1. 调用 uart3_init(115200) 初始化
 *   2. 在主循环中检查 USART3_RX_STA 获取接收数据
 *   3. 处理完成后清零 USART3_RX_STA
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef _USART3_H
#define _USART3_H
#include "sys.h"
#include "stdio.h"

#define USART3_REC_LEN  1024  // 最大接收字节数
#define EN_USART3_RX    1     // 使能(1)/禁止(0)串口3接收

extern u8  USART3_RX_BUF[USART3_REC_LEN]; // 接收缓冲区
extern u16 USART3_RX_STA;                 // 接收状态标记
extern UART_HandleTypeDef UART3_Handler;  // UART句柄

#define USART3_RXBUFFERSIZE   1            // HAL库接收缓冲区大小(单字节中断)
extern u8 aRxBuffer3[USART3_RXBUFFERSIZE]; // HAL库USART3接收Buffer

void uart3_init(u32 bound);  // 初始化USART3

#endif
