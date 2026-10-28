
#include <stdio.h>                                         // 引入标准IO头文件（此文件中未直接使用，可用于调试输出扩展）
#include "usart3.h"                                       // 引入USART3驱动头文件（函数声明与缓冲区声明）
 #include "esp8266.h"                                    // 引入ESP8266头文件（用于接收回调ESP8266_RxHandler）
                                                          // 预留空行占位（不影响功能）
char Usart3RecBuf[USART3_RXBUFF_SIZE];//串口3接收数据缓存    // USART3接收缓冲数组定义（保存接收到的数据）
unsigned int Rx3Counter = 0;   //串口3收到数据标志位          // USART3接收计数/下标（记录当前写入位置）

void USART3_Init(u32 baud)                                 // USART3初始化函数（baud为波特率）
 {                                                         // 函数体开始
      USART_InitTypeDef USART_InitStructure;               // 定义USART初始化结构体
      NVIC_InitTypeDef NVIC_InitStructure;                 // 定义NVIC初始化结构体
      GPIO_InitTypeDef GPIO_InitStructure;                 // 定义GPIO初始化结构体

      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE); // 使能GPIOB时钟（PB10/PB11）
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  // 使能USART3时钟（APB1总线）
                                                          // 预留空行占位
      // Configure USART3 Rx (PB.11) as input floating      // 配置USART3接收引脚PB11为浮空输入
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;           // 选择PB11引脚
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 设置为浮空输入模式
      GPIO_Init(GPIOB, &GPIO_InitStructure);               // 初始化GPIOB的PB11
                                                          // 预留空行占位
      // Configure USART3 Tx (PB.10) as alternate function push-pull  // 配置USART3发送引脚PB10为复用推挽输出
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;           // 选择PB10引脚
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    // 设置GPIO速度50MHz
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      // 设置为复用推挽输出模式
      GPIO_Init(GPIOB, &GPIO_InitStructure);               // 初始化GPIOB的PB10
                                                          // 预留空行占位
      USART_InitStructure.USART_BaudRate = baud;           // 设置USART3波特率
      USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 设置数据位长度为8位
      USART_InitStructure.USART_StopBits = USART_StopBits_1;      // 设置停止位为1位
      USART_InitStructure.USART_Parity = USART_Parity_No;         // 设置无校验
      USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 设置无硬件流控
      USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 使能收发模式
                                                          // 预留空行占位
                                                          // 预留空行占位
      // Configure USART3                                   // 配置USART3参数
      USART_Init(USART3, &USART_InitStructure);            // 初始化USART3（写入配置）
      // Enable USART3 Receive interrupts                   // 使能USART3接收中断
      USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);       // 使能接收非空中断
      // Enable the USART3                                  // 使能USART3外设
      USART_Cmd(USART3, ENABLE);                           // 使能USART3
                                                          // 预留空行占位
      //Configure the NVIC Preemption Priority Bits         // 配置NVIC优先级分组
      NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);      // 设置NVIC优先级分组为2
                                                          // 预留空行占位
      // Enable the USART3 Interrupt                        // 使能USART3中断通道
      NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;    // 选择USART3中断通道
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ; // 设置抢占优先级为1
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;   // 设置响应优先级为1
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      // 使能NVIC通道
      NVIC_Init(&NVIC_InitStructure);                      // 初始化NVIC配置
  }                                                        // 函数体结束
                                                         // 预留空行占位
void USART3_Sned_Char(u8 temp)                             // USART3发送单字节函数（轮询发送）
 {                                                         // 函数体开始
     USART_SendData(USART3,(u8)temp);                      // 向USART3数据寄存器写入1字节
    while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET); // 等待发送数据寄存器空（TXE置位）
 }                                                         // 函数体结束

void Uart3_SendStr(char*SendBuf)//串口3打印数据             // USART3发送字符串函数（以\0结束）
{                                                         // 函数体开始
	while(*SendBuf)                                        // 遍历字符串直到遇到结束符\0
	{                                                     // while循环体开始
	  USART3_Sned_Char(*SendBuf);                          // 发送当前字符
		SendBuf++;                                          // 指针后移
	}                                                     // while循环体结束
}                                                         // 函数体结束

void uart3_send(unsigned char *bufs,unsigned char len)      // USART3发送缓冲区函数
{                                                         // 函数体开始
	  if(len != 0xFF)                                      // 如果len不是0xFF，则按指定长度发送
		{                                                   // if分支开始
				while (len--)                                   // 循环发送len个字节
				{                                               // while循环体开始
						USART3_Sned_Char(*bufs);                   // 发送当前字节
						bufs ++;                                   // 指针后移
				}                                               // while循环体结束
		}                                                   // if分支结束
		else//如果字长等于或超过255，不按用户写入字长发送     // 否则：len=0xFF时按字符串方式发送
    {                                                   // else分支开始
        for (; *bufs != 0;	bufs++)USART3_Sned_Char(*bufs); 	//把字符逐个发送出去 // 循环直到遇到\0结束符并逐个发送
    }                                                   // else分支结束
}                                                         // 函数体结束

void USART3_IRQHandler(void)                               // USART3中断服务函数
{                                                         // 函数体开始
     if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  // 判断是否为接收非空中断
     {                                                     // if分支开始
			  uint8_t ch = (uint8_t)USART_ReceiveData(USART3);   // 读取接收数据寄存器得到1字节
			  if(Rx3Counter >= USART3_RXBUFF_SIZE)Rx3Counter=0;   // 防止缓冲区越界：到达末尾则回到0
			  Usart3RecBuf[Rx3Counter++] = ch;                    // 将接收到的字节存入缓冲区并递增计数
			  ESP8266_RxHandler(ch);                              // 将接收字节转交给ESP8266接收处理函数
                                                          // 预留空行占位
     }                                                     // if分支结束

     if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) == SET)  // 判断是否发生溢出错误ORE
     {                                                     // if分支开始
         USART_ClearFlag(USART3,USART_FLAG_ORE);            // 清除ORE错误标志
     }                                                     // if分支结束
     USART_ClearITPendingBit(USART3, USART_IT_RXNE);        // 清除接收非空中断挂起标志
}                                                         // 函数体结束


