/*
 * 模块名称: AT24CXX系列EEPROM驱动 (24cxx)
 * 功能概述:
 *   - 支持AT24C01~AT24C256全系列EEPROM
 *   - 提供单字节/多字节读写接口
 *   - 用于存储系统配置、校准数据等掉电保存信息
 * 
 * 硬件连接:
 *   - 通过软件IIC连接 (PB8-SCL, PB9-SDA)
 *   - 器件地址: 0xA0/0xA1
 * 
 * 本工程使用:
 *   - AT24C02 (256字节)
 *   - 用于存储触摸屏校准数据、离线事件缓存等
 * 
 * 使用方法:
 *   1. 调用 AT24CXX_Init() 初始化
 *   2. 调用 AT24CXX_Check() 检测EEPROM是否正常
 *   3. 使用 AT24CXX_ReadOneByte/WriteOneByte 读写单字节
 *   4. 使用 AT24CXX_Read/Write 读写多字节
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef _24CXX_H
#define _24CXX_H
#include "sys.h"
#include "myiic.h"

/*
 * AT24CXX系列容量定义
 * 数值表示最大地址(容量-1)
 */
#define AT24C01     127      // 128字节
#define AT24C02     255      // 256字节
#define AT24C04     511      // 512字节
#define AT24C08     1023     // 1K字节
#define AT24C16     2047     // 2K字节
#define AT24C32     4095     // 4K字节
#define AT24C64     8191     // 8K字节
#define AT24C128    16383    // 16K字节
#define AT24C256    32767    // 32K字节

/*
 * 当前使用的EEPROM型号
 * 本工程使用AT24C02 (256字节)
 */
#define EE_TYPE AT24C02

/*
 * 函数声明
 */
u8 AT24CXX_ReadOneByte(u16 ReadAddr);                           // 指定地址读取一个字节
void AT24CXX_WriteOneByte(u16 WriteAddr, u8 DataToWrite);       // 指定地址写入一个字节
void AT24CXX_WriteLenByte(u16 WriteAddr, u32 DataToWrite, u8 Len); // 指定地址写入指定长度数据
u32 AT24CXX_ReadLenByte(u16 ReadAddr, u8 Len);                  // 指定地址读取指定长度数据
void AT24CXX_Write(u16 WriteAddr, u8 *pBuffer, u16 NumToWrite); // 从指定地址写入指定长度数据
void AT24CXX_Read(u16 ReadAddr, u8 *pBuffer, u16 NumToRead);    // 从指定地址读取指定长度数据

u8 AT24CXX_Check(void);   // 检测EEPROM是否正常
void AT24CXX_Init(void);  // 初始化IIC接口

#endif
