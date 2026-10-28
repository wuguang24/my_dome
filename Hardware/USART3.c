#ifdef ENABLE_UART_PRINT

#include "USART3.h"
#include <string.h>

// USART3：本工程主要用作“调试输出串口”（printf/日志），只配置了发送（Tx）。
// 典型连接：PB10 -> USART3_TX（接 USB-TTL 的 RX），PB11 -> USART3_RX（如需接收可再补）。
void USART3_Init(uint32_t wBaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // GPIOB 属于 APB2；USART3 外设挂在 APB1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // USART3_TX PB10, USART3_RX PB11 (STM32F103C8T6 48脚可用)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 这里只初始化 Tx：USART_Mode_Tx
    // 若需要接收，可以补充 PB11 输入配置 + USART_Mode_Rx + RXNE 中断。

    USART_InitStructure.USART_BaudRate = wBaudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_Cmd(USART3, ENABLE);
}

void USART3_Send_Data(uint8_t *pbData, uint8_t bDataLen)
{
    uint8_t i;
    if (!pbData || bDataLen == 0) return;

    // 阻塞式发送：逐字节等待发送数据寄存器空（TXE），最后等待发送完成（TC）。
    // 注意：该函数会占用 CPU；适合调试输出，不建议在高频中断中调用。
    for (i = 0; i < bDataLen; i++)
    {
        // TXE=1：发送数据寄存器为空，可以写入新字节
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, pbData[i]);
    }
    // TC=1：最后一个字节已经从移位寄存器发送完毕（线路空闲）
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

#endif
