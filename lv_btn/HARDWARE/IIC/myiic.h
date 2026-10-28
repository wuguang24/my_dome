/*
 * 模块名称: 软件模拟IIC驱动 (myiic)
 * 功能概述:
 *   - 使用GPIO模拟IIC总线时序
 *   - 提供IIC通信的基本操作接口
 *   - 支持主机模式，可连接多种IIC从设备
 * 
 * 硬件连接:
 *   - SCL: PB8 (时钟线，推挽输出)
 *   - SDA: PB9 (数据线，开漏输出/输入切换)
 * 
 * 通信速率:
 *   - 标准模式约100kHz (由delay_us延时决定)
 * 
 * 使用方法:
 *   1. 调用 IIC_Init() 初始化GPIO
 *   2. 使用 IIC_Start() 发送起始信号
 *   3. 使用 IIC_Send_Byte() 发送数据
 *   4. 使用 IIC_Read_Byte() 接收数据
 *   5. 使用 IIC_Stop() 发送停止信号
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef _MYIIC_H
#define _MYIIC_H
#include "sys.h"

/*
 * SDA方向控制宏
 * IIC数据线需要在输入/输出模式间切换:
 * - 发送数据时: SDA配置为输出
 * - 接收数据/等待ACK时: SDA配置为输入
 */
#define SDA_IN()  {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=0<<9*2;}  // PB9输入模式
#define SDA_OUT() {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=1<<9*2;}  // PB9输出模式

/*
 * IIC引脚操作宏
 * 使用位带操作实现快速GPIO控制
 */
#define IIC_SCL   PBout(8)   // SCL时钟线 (PB8)
#define IIC_SDA   PBout(9)   // SDA数据线输出 (PB9)
#define READ_SDA  PBin(9)    // 读取SDA数据线 (PB9)

/*
 * IIC操作函数声明
 */
void IIC_Init(void);                 // 初始化IIC的IO口(PB8/PB9)
void IIC_Start(void);                // 产生IIC起始信号
void IIC_Stop(void);                 // 产生IIC停止信号
void IIC_Send_Byte(u8 txd);          // IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack); // IIC读取一个字节(ack=1发送ACK,ack=0发送NACK)
u8 IIC_Wait_Ack(void);               // IIC等待ACK信号(返回0成功,1失败)
void IIC_Ack(void);                  // IIC发送ACK应答
void IIC_NAck(void);                 // IIC发送NACK非应答

#endif
