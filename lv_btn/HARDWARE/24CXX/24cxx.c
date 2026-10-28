/*
 * 文件名称: 24cxx.c
 * 模块功能: AT24CXX系列EEPROM驱动实现
 * 
 * 功能说明:
 *   - 支持AT24C01~AT24C256全系列EEPROM
 *   - 提供单字节/多字节读写接口
 *   - 支持跨页写入(自动处理页边界)
 * 
 * AT24CXX系列容量:
 *   - AT24C01:  128字节 (1Kbit)   地址: 0~127
 *   - AT24C02:  256字节 (2Kbit)   地址: 0~255
 *   - AT24C04:  512字节 (4Kbit)   地址: 0~511
 *   - AT24C08:  1024字节 (8Kbit)  地址: 0~1023
 *   - AT24C16:  2048字节 (16Kbit) 地址: 0~2047
 *   - AT24C32:  4096字节 (32Kbit) 地址: 0~4095
 *   - AT24C64:  8192字节 (64Kbit) 地址: 0~8191
 *   - AT24C128: 16384字节(128Kbit)地址: 0~16383
 *   - AT24C256: 32768字节(256Kbit)地址: 0~32767
 * 
 * IIC地址说明:
 *   - 器件地址: 0xA0 (写) / 0xA1 (读)
 *   - AT24C01~AT24C16: 使用器件地址的A2/A1/A0位选择页
 *   - AT24C32及以上: 使用16位数据地址
 * 
 * 硬件连接:
 *   - 通过软件IIC (PB8-SCL, PB9-SDA) 连接
 *   - WP引脚接地(允许写入)
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 * 日期: 2025-12-12
 */

#include "24cxx.h"
#include "delay.h"

/**
 * @brief  初始化AT24CXX的IIC接口
 * @param  None
 * @retval None
 */
void AT24CXX_Init(void)
{
    IIC_Init();  // IIC初始化
}

/**
 * @brief  在AT24CXX指定地址读取一个字节
 * @param  ReadAddr: 读取数据的目的地址 (0~EE_TYPE)
 * @retval 读取到的数据
 * @note   读取流程:
 *         1. 发送起始信号
 *         2. 发送器件地址+写命令
 *         3. 发送数据地址
 *         4. 重新发送起始信号
 *         5. 发送器件地址+读命令
 *         6. 读取数据，发送NACK
 *         7. 发送停止信号
 */
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{
    u8 temp = 0;
    
    IIC_Start();
    
    if(EE_TYPE > AT24C16)
    {
        // AT24C32及以上容量，使用16位地址
        IIC_Send_Byte(0XA0);           // 发送写命令(器件地址)
        IIC_Wait_Ack();                // 等待应答
        IIC_Send_Byte(ReadAddr >> 8);  // 发送高地址
    }
    else
    {
        // AT24C01~AT24C16，地址编码在器件地址中
        IIC_Send_Byte(0XA0 + ((ReadAddr / 256) << 1));  // 发送器件地址+页地址
    }
    
    IIC_Wait_Ack();
    IIC_Send_Byte(ReadAddr % 256);     // 发送低地址
    IIC_Wait_Ack();
    
    IIC_Start();                       // 重新发送起始信号
    IIC_Send_Byte(0XA1);               // 进入读模式
    IIC_Wait_Ack();
    temp = IIC_Read_Byte(0);           // 读取数据，发送NACK
    IIC_Stop();                        // 产生停止信号
    
    return temp;
}

/**
 * @brief  在AT24CXX指定地址写入一个字节
 * @param  WriteAddr: 写入数据的目的地址
 * @param  DataToWrite: 要写入的数据
 * @retval None
 * @note   写入后需要等待约10ms让EEPROM完成内部写入
 *         这是EEPROM的特性，每次写入都需要等待
 */
void AT24CXX_WriteOneByte(u16 WriteAddr, u8 DataToWrite)
{
    IIC_Start();
    
    if(EE_TYPE > AT24C16)
    {
        // AT24C32及以上容量，使用16位地址
        IIC_Send_Byte(0XA0);           // 发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr >> 8); // 发送高地址
    }
    else
    {
        // AT24C01~AT24C16
        IIC_Send_Byte(0XA0 + ((WriteAddr / 256) << 1));
    }
    
    IIC_Wait_Ack();
    IIC_Send_Byte(WriteAddr % 256);    // 发送低地址
    IIC_Wait_Ack();
    IIC_Send_Byte(DataToWrite);        // 发送字节
    IIC_Wait_Ack();
    IIC_Stop();                        // 产生停止信号
    
    delay_ms(10);                      // 等待EEPROM完成写入(重要!)
}

/**
 * @brief  在AT24CXX指定地址写入长度为Len的数据
 * @param  WriteAddr: 开始写入的地址
 * @param  DataToWrite: 要写入的数据(32位)
 * @param  Len: 要写入数据的长度(2或4字节)
 * @retval None
 * @note   用于写入16bit或32bit数据，低字节在前(小端模式)
 */
void AT24CXX_WriteLenByte(u16 WriteAddr, u32 DataToWrite, u8 Len)
{
    u8 t;
    for(t = 0; t < Len; t++)
    {
        AT24CXX_WriteOneByte(WriteAddr + t, (DataToWrite >> (8 * t)) & 0xff);
    }
}

/**
 * @brief  在AT24CXX指定地址读取长度为Len的数据
 * @param  ReadAddr: 开始读取的地址
 * @param  Len: 要读取数据的长度(2或4字节)
 * @retval 读取到的数据(32位)
 * @note   用于读取16bit或32bit数据，低字节在前(小端模式)
 */
u32 AT24CXX_ReadLenByte(u16 ReadAddr, u8 Len)
{
    u8 t;
    u32 temp = 0;
    
    for(t = 0; t < Len; t++)
    {
        temp <<= 8;
        temp += AT24CXX_ReadOneByte(ReadAddr + Len - t - 1);
    }
    return temp;
}

/**
 * @brief  检测AT24CXX是否正常
 * @param  None
 * @retval 0: 检测成功
 *         1: 检测失败
 * @note   通过在地址255写入标志字节0x55来检测
 *         首次使用时会写入标志，之后读取验证
 */
u8 AT24CXX_Check(void)
{
    u8 temp;
    
    temp = AT24CXX_ReadOneByte(255);   // 读取标志位置
    
    if(temp == 0X55)
        return 0;                      // 已经初始化过，检测成功
    else
    {
        // 首次初始化，写入标志
        AT24CXX_WriteOneByte(255, 0X55);
        temp = AT24CXX_ReadOneByte(255);
        if(temp == 0X55)
            return 0;                  // 写入成功
    }
    return 1;                          // 检测失败
}

/**
 * @brief  在AT24CXX指定地址开始读取指定个数的数据
 * @param  ReadAddr: 开始读取的地址 (AT24C02为0~255)
 * @param  pBuffer: 数据存储区首地址
 * @param  NumToRead: 要读取的数据个数
 * @retval None
 */
void AT24CXX_Read(u16 ReadAddr, u8 *pBuffer, u16 NumToRead)
{
    while(NumToRead)
    {
        *pBuffer++ = AT24CXX_ReadOneByte(ReadAddr++);
        NumToRead--;
    }
}

/**
 * @brief  在AT24CXX指定地址开始写入指定个数的数据
 * @param  WriteAddr: 开始写入的地址 (AT24C02为0~255)
 * @param  pBuffer: 数据存储区首地址
 * @param  NumToWrite: 要写入的数据个数
 * @retval None
 * @note   每写入一个字节都会等待10ms，大量写入时较慢
 *         如需提高效率，可使用页写入模式(需处理页边界)
 */
void AT24CXX_Write(u16 WriteAddr, u8 *pBuffer, u16 NumToWrite)
{
    while(NumToWrite--)
    {
        AT24CXX_WriteOneByte(WriteAddr, *pBuffer);
        WriteAddr++;
        pBuffer++;
    }
}
