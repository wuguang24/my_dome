/*
 * 模块名称: 电容触摸屏IIC驱动 (ctiic)
 * 功能概述:
 *   - 使用GPIO模拟IIC总线时序
 *   - 专用于电容触摸屏芯片通信
 *   - 支持GT1151/GT9147/FT5206等触摸IC
 * 
 * 硬件连接:
 *   - SCL: PB15 (时钟线，推挽输出)
 *   - SDA: PB12 (数据线，开漏输出/输入切换)
 * 
 * 与myiic的区别:
 *   - myiic用于EEPROM等通用IIC设备 (PB8/PB9)
 *   - ctiic专用于触摸屏 (PB15/PB12)
 *   - 时序略有不同，适配触摸IC特性
 * 
 * 使用方法:
 *   1. 调用 CT_IIC_Init() 初始化GPIO
 *   2. 使用 CT_IIC_Start/Stop/Send_Byte/Read_Byte 进行通信
 *   3. 通常由GT1151等触摸驱动调用，不直接使用
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef __MYCT_IIC_H
#define __MYCT_IIC_H
#include "sys.h"

/*
 * SDA方向控制宏
 * 触摸屏IIC数据线需要在输入/输出模式间切换
 */
#define CT_SDA_IN()  {GPIOB->MODER&=~(3<<(2*12));GPIOB->MODER|=0<<2*12;}  // PB12输入模式
#define CT_SDA_OUT() {GPIOB->MODER&=~(3<<(2*12));GPIOB->MODER|=1<<2*12;}  // PB12输出模式

/*
 * IIC引脚操作宏
 */
#define CT_IIC_SCL    PBout(15)   // SCL时钟线 (PB15)
#define CT_IIC_SDA    PBout(12)   // SDA数据线输出 (PB12)
#define CT_READ_SDA   PBin(12)    // 读取SDA数据线 (PB12)

/*
 * IIC操作函数声明
 */
void CT_IIC_Init(void);                  // 初始化IIC的IO口
void CT_IIC_Start(void);                 // 产生IIC起始信号
void CT_IIC_Stop(void);                  // 产生IIC停止信号
void CT_IIC_Send_Byte(u8 txd);           // IIC发送一个字节
u8 CT_IIC_Read_Byte(unsigned char ack);  // IIC读取一个字节
u8 CT_IIC_Wait_Ack(void);                // IIC等待ACK信号
void CT_IIC_Ack(void);                   // IIC发送ACK应答
void CT_IIC_NAck(void);                  // IIC发送NACK非应答

#endif
