
 #include <stm32f10x.h>                                  // 引入STM32F10x外设库头文件（USART/GPIO/RCC等）
#include <stdio.h>                                       // 引入标准IO头文件（用于printf相关重定向）
#include "usart1.h"                                     // 引入USART1驱动头文件（函数与缓冲区声明）

char Usart1RecBuf[USART1_RXBUFF_SIZE];//串口1接收数据缓存  // USART1接收缓冲数组定义（保存接收到的数据）
unsigned int RxCounter = 0;   //串口1收到数据标志位        // USART1接收计数/下标（记录当前写入位置）

//////////////////////////////////////////////////////////////////            // 分隔线：下方为printf重定向相关代码
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1                                                   // 条件编译：使能printf重定向代码块
#pragma import(__use_no_semihosting)                    // 禁用半主机模式（避免依赖调试器）
//标准库需要的支持函数                                   // 注释：标准库printf需要的底层支持结构
struct __FILE                                           // 定义FILE结构体（用于重定向stdout）
{                                                       // 结构体开始
	int handle;                                          // 句柄成员（占位）

};                                                      // 结构体结束

FILE __stdout;                                          // 定义标准输出对象
//定义_sys_exit()以避免使用半主机模式                    // 注释：避免链接到半主机退出函数
void _sys_exit(int x)                                   // 系统退出函数（防止程序异常退出）
{                                                       // 函数体开始
	(void)x;  // 避免未使用参数警告                        // 显式丢弃参数，避免编译器告警
	// 无限循环，防止程序退出                              // 注释：不允许返回到上层
	while(1)                                              // 死循环
	{                                                     // 循环体开始
		// 可以选择在这里添加看门狗重置或其他处理            // 注释：可扩展逻辑
	}                                                     // 循环体结束
}                                                       // 函数体结束

void _ttywrch(int ch)                                   // 低层输出字符函数（此处占位）
{                                                       // 函数体开始
	(void)ch;                                             // 丢弃参数，避免未使用告警
}                                                       // 函数体结束
//重定义fputc函数                                        // 注释：把fputc输出重定向到USART1
int fputc(int ch, FILE *f)                              // fputc重定向实现（支持printf）
{                                                       // 函数体开始
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕     // 等待发送完成（TC标志位）
    USART1->DR = (u8) ch;                               // 写数据寄存器发送1字节
	return ch;                                            // 返回已发送字符
}                                                       // 函数体结束

#endif                                                  // 条件编译结束



/****************************************************************************  // 文件说明：USART1初始化函数注释块开始

描述：Usart1 初始化                                     // 描述：初始化USART1
   PA9： USART1_TX                                      // 原注释：TX引脚说明（但当前代码使用PB3）
   PA10: USART1_RX                                      // 原注释：RX引脚说明（但当前代码使用PB4）

参数：none                                              // 参数：无（实际传入bound为波特率）

返回: none                                              // 返回：无

****************************************************************************/ // 文件说明：USART1初始化函数注释块结束

void uart1_Init(u32 bound)                               // USART1初始化函数（bound为波特率）
{                                                       // 函数体开始
    //GPIO端口设置                                      // 注释：下面进行GPIO与USART配置
  GPIO_InitTypeDef GPIO_InitStructure;                   // 定义GPIO初始化结构体
  USART_InitTypeDef USART_InitStructure;                 // 定义USART初始化结构体
  NVIC_InitTypeDef NVIC_InitStructure;	                // 定义NVIC初始化结构体
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE); // 使能USART1/GPIOB/AFIO时钟
	
	USART_DeInit(USART1);                                  // 复位USART1寄存器到默认状态
 
     //USART1_TX   PA.9                                 // 注释：原注释（此处实际配置为PB3复用推挽输出）
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;              // 选择PB3作为USART1_TX引脚
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // 设置GPIO速度50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;        // 配置为复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);                 // 初始化GPIOB的PB3
   
    //USART1_RX	  PA.10                                // 注释：原注释（此处实际配置为PB4浮空输入）
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;              // 选择PB4作为USART1_RX引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // 配置为浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);                 // 初始化GPIOB的PB4

   //Usart1 NVIC 配置                                   // 注释：配置USART1中断优先级

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;      // 选择USART1中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ; // 抢占优先级设置为0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		 // 响应优先级设置为1

  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//IRQ通道使能 // 使能USART1中断通道
  NVIC_Init(&NVIC_InitStructure);	//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器USART1 // 初始化NVIC配置
  
   //USART 初始化设置                                   // 注释：设置USART波特率/数据位/停止位等
  USART_InitStructure.USART_BaudRate = bound;//一般设置为9600; // 设置波特率（由bound参数指定）
  USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 数据位8位
  USART_InitStructure.USART_StopBits = USART_StopBits_1;      // 停止位1位
  USART_InitStructure.USART_Parity = USART_Parity_No;         // 无校验
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 使能收发模式

  USART_Init(USART1, &USART_InitStructure);              // 配置USART1参数（重复调用一次也不影响）
   

  USART_Init(USART1, &USART_InitStructure);     //初始化串口1 // 初始化USART1（再次调用）
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口1接受中断 // 使能接收非空中断
  USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);//使能串口1总线空闲中断/ // 禁用空闲中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1  // 使能USART1外设
}                                                       // 函数体结束


void USART1_IRQHandler(void)                	//串口1中断服务程序 // USART1中断服务函数
{                                                       // 函数体开始
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断，可以扩展来控制 // 判断是否为接收非空中断
		{                                                     // if分支开始
				Usart1RecBuf[RxCounter++] =USART_ReceiveData(USART1);//接收模块的数据 // 读取接收数据寄存器并存入缓冲区
    }                                                    // if分支结束
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);        // 清除接收非空中断挂起标志
}                                                        // 函数体结束

void uart1_SendStr(char*SendBuf)//串口1打印数据           // USART1发送字符串函数（轮询发送）
{                                                       // 函数体开始
	while(*SendBuf)                                        // 遍历字符串直到遇到结束符\0
	{                                                     // while循环体开始
	  while((USART1->SR&0X40)==0);//等待发送完成            // 等待发送完成（TC标志位）
    USART1->DR = (u8) *SendBuf;                          // 发送当前字符
		SendBuf++;                                          // 指针后移到下一个字符
	}                                                     // while循环体结束
}                                                       // 函数体结束

void uart1_send(unsigned char *bufs,unsigned char len)    // USART1发送缓冲区函数
{                                                       // 函数体开始
	  if(len != 0xFF)                                      // 如果len不是0xFF，则按指定长度发送
		{                                                   // if分支开始
				while (len--)                                   // 循环发送len个字节
				{                                               // while循环体开始
						while((USART1->SR&0X40)==0);//等待发送完成  // 等待发送完成（TC标志位）
						USART1->DR = (u8) *bufs;                   // 发送当前字节
						bufs ++;                                   // 指针后移
				}                                               // while循环体结束
		}                                                   // if分支结束
		else//如果字长等于或超过255，不按用户写入字长发送     // 否则：len=0xFF时按字符串方式发送
    {                                                   // else分支开始
        for (; *bufs != 0;	bufs++) 	//把字符逐个发送出去 // 循环直到遇到\0结束符
			  {                                               // for循环体开始
						while((USART1->SR&0X40)==0);//等待发送完成  // 等待发送完成（TC标志位）
						USART1->DR = (u8) *bufs;                   // 发送当前字节
				}                                               // for循环体结束
    }                                                   // else分支结束
}                                                       // 函数体结束


