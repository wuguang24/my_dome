/*
 * 文件名称: usart3.c
 * 模块功能: USART3串口驱动(雷达通信)
 * 功能说明:
 *   - 配置USART3用于雷达模块通信
 *   - 支持中断接收
 *   - 接收缓冲区管理
 * 硬件连接:
 *   - USART3_TX: PB10
 *   - USART3_RX: PB11
 *   - 波特率: 115200bps
 * 通信对象:
 *   - 雷达模块(人体检测传感器)
 * 版本: V1.0
 * 日期: 2025-12-02
 */

#include "usart3.h"
#include "delay.h"

#if EN_USART3_RX   // 如果使能了接收

/* 串口3接收缓冲区 */
u8 USART3_RX_BUF[USART3_REC_LEN];     // 接收缓冲区，最大USART3_REC_LEN字节

/* 接收状态标志
 * bit15: 接收完成标志 (1=接收完成)
 * bit14: 接收到0x0d (回车符)
 * bit13~0: 接收到的有效字节数目
 */
u16 USART3_RX_STA = 0;                // 接收状态标记

u8 aRxBuffer3[USART3_RXBUFFERSIZE];   // HAL库使用的串口接收缓冲区
UART_HandleTypeDef UART3_Handler;      // UART句柄

/**
 * @brief  初始化USART3串口
 * @param  bound: 波特率(通常为115200)
 * @retval None
 * @note   USART3用于雷达模块通信
 *         引脚: PB10(TX), PB11(RX)
 *         配置: 8位数据，1位停止位，无校验
 */
void uart3_init(u32 bound)
{
    // UART 初始化参数
    UART3_Handler.Instance = USART3;                      // USART3
    UART3_Handler.Init.BaudRate = bound;                   // 波特率
    UART3_Handler.Init.WordLength = UART_WORDLENGTH_8B;    // 字长为8位数据格式
    UART3_Handler.Init.StopBits = UART_STOPBITS_1;         // 一个停止位
    UART3_Handler.Init.Parity = UART_PARITY_NONE;          // 无奇偶校验位
    UART3_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;    // 无硬件流控
    UART3_Handler.Init.Mode = UART_MODE_TX_RX;             // 收发模式
    HAL_UART_Init(&UART3_Handler);                         // 初始化UART3(会自动使能)

    // 开启接收中断，设置接收缓冲区
    HAL_UART_Receive_IT(&UART3_Handler, (u8 *)aRxBuffer3, USART3_RXBUFFERSIZE);
}

// 注意：HAL_UART_MspInit在stm32f4xx_hal_msp.c中实现(GPIO配置、时钟使能、中断配置)

// 注意：HAL_UART_RxCpltCallback在stm32f4xx_hal_msp.c中统一实现(接收完成回调)

/**
 * @brief  USART3中断服务函数
 * @param  None
 * @retval None
 * @note   接收到数据时自动触发
 *         处理流程:
 *         1. 调用HAL库中断处理函数
 *         2. 等待UART就绪
 *         3. 重新开启接收中断
 */
void USART3_IRQHandler(void)
{
    u32 timeout = 0;                                       // 超时计数器

    HAL_UART_IRQHandler(&UART3_Handler);                   // HAL库中断处理函数

    /* 等待UART就绪 */
    timeout = 0;
    while (HAL_UART_GetState(&UART3_Handler) != HAL_UART_STATE_READY)
    {
        timeout++;                                         // 超时计数
        if (timeout > HAL_MAX_DELAY) break;                // 超时退出
    }

    /* 重新开启接收中断 */
    timeout = 0;
    while (HAL_UART_Receive_IT(&UART3_Handler, (u8 *)aRxBuffer3, USART3_RXBUFFERSIZE) != HAL_OK)
    {
        timeout++;                                         // 超时计数
        if (timeout > HAL_MAX_DELAY) break;                // 超时退出
    }
}
#endif

