/*
 * 模块名称: UART4串口驱动 (uart4)
 * 功能概述:
 *   - 配置UART4用于GPS模块通信
 *   - 支持中断接收，缓冲区管理
 * 
 * 硬件连接:
 *   - UART4_TX: PC10
 *   - UART4_RX: PC11
 *   - 波特率: 9600bps (GPS标准波特率)
 * 
 * 通信对象:
 *   - GPS模块
 *   - 协议: NMEA 0183 (ASCII文本协议)
 * 
 * 缓冲区说明:
 *   - UART4_RX_BUF[]: 接收缓冲区，最大1024字节
 *   - UART4_RX_STA: 接收状态标记
 *     - bit15: 接收完成标志
 *     - bit14: 接收到0x0d(回车符)
 *     - bit13~0: 接收到的有效字节数目
 * 
 * 使用方法:
 *   1. 调用 uart4_init(9600) 初始化
 *   2. 在主循环中检查 UART4_RX_STA 获取接收数据
 *   3. 处理完成后清零 UART4_RX_STA
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef _UART4_H
#define _UART4_H
#include "sys.h"
#include "stdio.h"

#define UART4_REC_LEN  1024  // 最大接收字节数
#define EN_UART4_RX    1     // 使能(1)/禁止(0)串口4接收

extern u8  UART4_RX_BUF[UART4_REC_LEN]; // 接收缓冲区
extern u16 UART4_RX_STA;                // 接收状态标记
extern UART_HandleTypeDef UART4_Handler; // UART句柄

#define UART4_RXBUFFERSIZE   1            // HAL库接收缓冲区大小(单字节中断)
extern u8 aRxBuffer4[UART4_RXBUFFERSIZE]; // HAL库UART4接收Buffer

void uart4_init(u32 bound);  // 初始化UART4

#endif
