#include "gy302.h"
#include "MyI2C.h"
#include "Delay.h"

// GY-302 光照传感器模块（BH1750）
// 通信：I2C（本工程使用 MyI2C 软件 I2C）
// 常用模式：连续高分辨率模式（BH1750_CONT_H_MODE）
// 注意：
// - BH1750 的测量需要一定时间，H 模式典型 120ms~180ms，本文件使用 180ms 保守等待
// - 光照换算：Lux = raw / 1.2（datasheet 给出）

/**
  * @brief  向BH1750发送命令
  * @param  cmd 命令字节
  * @retval 0：成功，1：失败
  */
static uint8_t BH1750_SendCmd(uint8_t cmd)
{
    // 写流程：Start -> 地址(写) -> cmd -> Stop
    MyI2C_Start();
    MyI2C_SendByte(BH1750_ADDR << 1);  // 写地址
    if(MyI2C_ReceiveAck() != 0)
    {
        MyI2C_Stop();
        return 1;  // 失败
    }
    MyI2C_SendByte(cmd);
    if(MyI2C_ReceiveAck() != 0)
    {
        MyI2C_Stop();
        return 1;  // 失败
    }
    MyI2C_Stop();
    return 0;  // 成功
}

/**
  * @brief  GY-302传感器初始化
  */
void GY302_Init(void)
{
    // 上电
    BH1750_SendCmd(BH1750_POWER_ON);
    Delay_ms(10);
    
    // 复位
    BH1750_SendCmd(BH1750_RESET);
    Delay_ms(10);
    
    // 设置连续高分辨率模式
    BH1750_SendCmd(BH1750_CONT_H_MODE);
    Delay_ms(180);  // 等待第一次测量完成
}

/**
  * @brief  读取光照强度
  * @retval 光照强度值（lx），失败返回0.0
  */
float GY302_ReadLight(void)
{
    uint8_t data_h, data_l;
    uint16_t raw_data;
    
    // 确保处于测量模式（连续 H 模式会持续刷新测量结果）
    BH1750_SendCmd(BH1750_CONT_H_MODE);
    Delay_ms(180);  // 等待测量完成
    
    // 开始读取数据（2 字节）
    MyI2C_Start();
    MyI2C_SendByte((BH1750_ADDR << 1) | 0x01);  // 读地址
    
    // 检查应答
    if(MyI2C_ReceiveAck() != 0)
    {
        MyI2C_Stop();
        return 0.0;
    }
    
    // 读取高字节（读完后发送 ACK，表示还要继续读）
    data_h = MyI2C_ReceiveByte();
    MyI2C_SendAck(0);  // 应答
    
    // 读取低字节（最后一个字节发送 NACK，表示停止读取）
    data_l = MyI2C_ReceiveByte();
    MyI2C_SendAck(1);  // 非应答（结束读取）
    
    MyI2C_Stop();
    
    // 合并数据（MSB first）
    raw_data = (data_h << 8) | data_l;
    
    // 数据有效性检查
    if(raw_data == 0 || raw_data == 0xFFFF)
        return 0.0;
    
    // 转换公式：光照强度 = 原始数据 / 1.2
    return (float)raw_data / 1.2;
}
