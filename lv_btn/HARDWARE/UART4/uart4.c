/*
 * 文件名称: uart4.c
 * 模块功能: UART4串口驱动(GPS通信)
 * 功能说明:
 *   - 配置UART4用于GPS模块通信
 *   - 支持中断接收
 *   - 接收缓冲区管理
 * 硬件连接:
 *   - UART4_TX: PC10
 *   - UART4_RX: PC11
 *   - 波特率: 9600bps (GPS标准波特率)
 * 通信对象:
 *   - GPS模块(NMEA协议)
 * 版本: V1.0
 * 日期: 2025-12-02
 */

#include "uart4.h"
#include "delay.h"

#if EN_UART4_RX   // 如果使能了接收

/* 串口4接收缓冲区 */
u8 UART4_RX_BUF[UART4_REC_LEN];     // 接收缓冲区，最大UART4_REC_LEN字节

/* 接收状态标志
 * bit15: 接收完成标志 (1=接收完成)
 * bit14: 接收到0x0d (回车符)
 * bit13~0: 接收到的有效字节数目
 */
u16 UART4_RX_STA = 0;                // 接收状态标记

u8 aRxBuffer4[UART4_RXBUFFERSIZE];   // HAL库使用的串口接收缓冲区
UART_HandleTypeDef UART4_Handler;    // UART句柄

/**
 * @brief  初始化UART4串口
 * @param  bound: 波特率(GPS通常为9600)
 * @retval None
 * @note   UART4用于GPS模块通信
 *         引脚: PC10(TX), PC11(RX)
 *         配置: 8位数据，1位停止位，无校验
 *         协议: NMEA 0183
 */
void uart4_init(u32 bound)
{
    /* UART初始化参数 */
    UART4_Handler.Instance = UART4;                      // UART4外设
    UART4_Handler.Init.BaudRate = bound;                 // 波特率(GPS标准9600)
    UART4_Handler.Init.WordLength = UART_WORDLENGTH_8B;  // 8位数据位
    UART4_Handler.Init.StopBits = UART_STOPBITS_1;       // 1位停止位
    UART4_Handler.Init.Parity = UART_PARITY_NONE;        // 无校验位
    UART4_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // 无硬件流控
    UART4_Handler.Init.Mode = UART_MODE_TX_RX;           // 收发模式
    HAL_UART_Init(&UART4_Handler);                       // 初始化UART4(会自动使能)

    // 开启接收中断，设置接收缓冲区
    HAL_UART_Receive_IT(&UART4_Handler, (u8 *)aRxBuffer4, UART4_RXBUFFERSIZE);
}

// 注意：HAL_UART_MspInit在stm32f4xx_hal_msp.c中实现(GPIO配置、时钟使能、中断配置)

// 注意：HAL_UART_RxCpltCallback在stm32f4xx_hal_msp.c中统一实现(接收完成回调)

/**
 * @brief  UART4中断服务函数
 * @param  None
 * @retval None
 * @note   接收到GPS数据时自动触发
 *         处理流程:
 *         1. 调用HAL库中断处理函数
 *         2. 等待UART就绪
 *         3. 重新开启接收中断
 */
void UART4_IRQHandler(void)
{
    u32 timeout = 0;                                     // 超时计数器

    HAL_UART_IRQHandler(&UART4_Handler);                 // HAL库中断处理函数

    /* 等待UART就绪 */
    timeout = 0;
    while (HAL_UART_GetState(&UART4_Handler) != HAL_UART_STATE_READY)
    {
        timeout++;                                       // 超时计数
        if (timeout > HAL_MAX_DELAY) break;              // 超时退出
    }

    /* 重新开启接收中断 */
    timeout = 0;
    while (HAL_UART_Receive_IT(&UART4_Handler, (u8 *)aRxBuffer4, UART4_RXBUFFERSIZE) != HAL_OK)
    {
        timeout++;                                       // 超时计数
        if (timeout > HAL_MAX_DELAY) break;              // 超时退出
    }
}
#endif

