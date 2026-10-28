#ifndef __MYI2C_H
#define __MYI2C_H

#include "stm32f10x.h"

// I2C通信函数声明

// I2C初始化函数
void MyI2C_Init(void);
// 发送I2C开始信号
void MyI2C_Start(void);
// 发送I2C停止信号
void MyI2C_Stop(void);
// 发送一个字节
// 参数：Byte - 要发送的字节
void MyI2C_SendByte(uint8_t Byte);
// 接收一个字节
// 返回值：接收到的字节
uint8_t MyI2C_ReceiveByte(void);
// 发送应答位
// 参数：AckBit - 应答位（0：应答，1：非应答）
void MyI2C_SendAck(uint8_t AckBit);
// 接收应答位
// 返回值：应答位（0：应答，1：非应答）
uint8_t MyI2C_ReceiveAck(void);

#endif
