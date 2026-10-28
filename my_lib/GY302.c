#include "gy302.h"
#include "MyI2C.h"
#include "Delay.h"

/**
  * @brief  向BH1750发送命令
  * @param  cmd 命令字节
  * @retval 0成功，1失败
  */
static uint8_t BH1750_SendCmd(uint8_t cmd)
{
    MyI2C_Start();  // 发送I2C开始信号
    MyI2C_SendByte(BH1750_ADDR << 1);  // 发送写地址
    if(MyI2C_ReceiveAck() != 0)  // 检查应答
    {
        MyI2C_Stop();  // 发送I2C停止信号
        return 1;  // 失败
    }
    MyI2C_SendByte(cmd);  // 发送命令
    if(MyI2C_ReceiveAck() != 0)  // 检查应答
    {
        MyI2C_Stop();  // 发送I2C停止信号
        return 1;  // 失败
    }
    MyI2C_Stop();  // 发送I2C停止信号
    return 0;  // 成功
}

/**
  * @brief  GY-302光照传感器初始化
  */
void GY302_Init(void)
{
    // 唤醒传感器
    BH1750_SendCmd(BH1750_POWER_ON);
    Delay_ms(10);  // 延时10ms
    
    // 重置传感器
    BH1750_SendCmd(BH1750_RESET);
    Delay_ms(10);  // 延时10ms
    
    // 设置为连续高分辨率模式
    BH1750_SendCmd(BH1750_CONT_H_MODE);
    Delay_ms(180);  // 等待第一次测量完成
}

/**
  * @brief  读取光照强度
  * @retval 光照强度值（lx），读取失败返回0.0
  */
float GY302_ReadLight(void)
{
    uint8_t data_h, data_l;  // 高8位和低8位数据
    uint16_t raw_data;  // 原始数据
    
    // 确保传感器在测量模式
    BH1750_SendCmd(BH1750_CONT_H_MODE);
    Delay_ms(180);  // 等待测量完成
    
    // 开始读取数据
    MyI2C_Start();  // 发送I2C开始信号
    MyI2C_SendByte((BH1750_ADDR << 1) | 0x01);  // 发送读地址
    
    // 检查应答
    if(MyI2C_ReceiveAck() != 0)
    {
        MyI2C_Stop();  // 发送I2C停止信号
        return 0.0;  // 读取失败
    }
    
    // 读取高8位
    data_h = MyI2C_ReceiveByte();
    MyI2C_SendAck(0);  // 发送应答
    
    // 读取低8位
    data_l = MyI2C_ReceiveByte();
    MyI2C_SendAck(1);  // 发送非应答，结束数据读取
    
    MyI2C_Stop();  // 发送I2C停止信号
    
    // 组合数据
    raw_data = (data_h << 8) | data_l;
    
    // 检查数据有效性
    if(raw_data == 0 || raw_data == 0xFFFF)
        return 0.0;  // 无效数据
    
    // 转换格式：光照强度 = 原始数据 / 1.2
    return (float)raw_data / 1.2;
}
