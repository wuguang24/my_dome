#include "USART1.h"
#include "string.h"

// USART1 串口驱动（本工程主要用于 JW01 CO2 模块数据接收）
// 硬件连接（默认 STM32F103）：
// - PA9  : USART1_TX
// - PA10 : USART1_RX
// 说明：本文件提供一个简单的“线性接收缓冲区”，在 RXNE 中断中逐字节累积。
// 上层在合适的时机读取 `USART1RecBuff` 并调用 Clear() 清空。

#define USART1_REC_MAX_LEN  (512)
uint16_t USART1RecLen = 0;
uint8_t USART1RecBuff[USART1_REC_MAX_LEN] = {0};

/**
 * @brief USART1发送一个字节数据
 *
 * @param bData
 */
void USART1_Send_Byte(uint8_t bData)
{
    // 阻塞发送：等待发送完成标志（TC/0x40）
    while((USART1->SR & 0X40) == 0);  //循环发送,直到发送完毕
    USART1->DR = bData;
}

/**
 * @brief USART1发送数据
 *
 * @param pbData
 * @param bDataLen
 */
void USART1_Send_Data(uint8_t *pbData, uint8_t bDataLen)
{
    // 逐字节阻塞发送
    for(uint8_t i = 0; i < bDataLen; ++i)
    {
        USART1_Send_Byte(pbData[i]);
    }
}

/**
 * @brief USART1中断服务函数
 *
 */
void USART1_IRQHandler(void)
{
    // RXNE：接收数据寄存器非空中断
    if(USART_GetITStatus(USART1, USART_IT_RXNE))  //中断标志
    {
        // 读 DR 会清 RXNE 标志（标准库封装）
        uint8_t bData = USART_ReceiveData(USART1);  //串口1接收
        USART1RecBuff[USART1RecLen++] = bData;
        if(USART1RecLen >= USART1_REC_MAX_LEN)
        {
            // 缓冲区越界保护：直接清空重新接收（避免写越界）
            USART1_Data_Clear();  //接收数据错误,重新开始接收
        }
    }
}

/**
 * @brief USART1初始化
 *
 * @param wBaudRate
 */
void USART1_Init(uint32_t wBaudRate)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    //使能USART1，GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    //USART1_TX  GPIOA9：复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  //PA9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化GPIOA9
    //USART1_RX  GPIOA10：浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  //PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化GPIOA10
    //USAR1T 初始化设置
    USART_InitStructure.USART_BaudRate = wBaudRate;  //串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  //字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;  //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  //收发模式
    USART_Init(USART1, &USART_InitStructure);  //初始化串口1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  //开启串口接收中断
    USART_Cmd(USART1, ENABLE);  //使能串口1
    //USART1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);  //根据指定的参数初始化VIC寄存器
}

/**
 * @brief USART1接收的数据清除
 *
 */
void USART1_Data_Clear(void)
{
    USART1RecLen = 0;
    memset(USART1RecBuff, 0, sizeof(USART1RecBuff));
}

/**
 * @brief USART1接收的数据获取
 *
 * @return uint8_t*
 */
uint8_t *USART1_Data_Get(void)
{
    return USART1RecBuff;
}

/**
 * @brief USART1接收的数据长度获取
 *
 * @return uint16_t
 */
uint16_t USART1_Data_Len_Get(void)
{
    return USART1RecLen;
}
