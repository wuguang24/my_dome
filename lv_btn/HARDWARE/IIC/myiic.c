/*
 * 文件名称: myiic.c
 * 模块功能: 软件模拟IIC驱动实现
 * 
 * 功能说明:
 *   - 使用GPIO模拟IIC总线时序
 *   - 实现IIC协议的起始、停止、发送、接收、应答等操作
 *   - 支持标准模式(约100kHz)通信速率
 * 
 * IIC协议简介:
 *   IIC(Inter-Integrated Circuit)是一种两线式串行总线协议:
 *   - SCL: 时钟线，由主机控制
 *   - SDA: 数据线，双向传输
 *   
 *   关键时序:
 *   - 起始信号: SCL高电平期间，SDA由高变低
 *   - 停止信号: SCL高电平期间，SDA由低变高
 *   - 数据有效: SCL高电平期间，SDA数据稳定
 *   - 数据变化: SCL低电平期间，SDA可以变化
 *   - ACK应答: 接收方在第9个时钟拉低SDA表示应答
 * 
 * 硬件连接:
 *   - SCL: PB8 (推挽输出)
 *   - SDA: PB9 (推挽输出，需要时切换为输入)
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 * 日期: 2025-12-12
 */

#include "myiic.h"
#include "delay.h"

/**
 * @brief  初始化IIC总线GPIO
 * @param  None
 * @retval None
 * @note   配置PB8(SCL)和PB9(SDA)为推挽输出，上拉，高速
 *         初始化后SCL和SDA都为高电平(空闲状态)
 */
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();   // 使能GPIOB时钟
    
    // PB8(SCL),PB9(SDA)初始化设置
    GPIO_Initure.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;          // 上拉
    GPIO_Initure.Speed = GPIO_SPEED_FAST;     // 快速
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
    
    // 初始化为空闲状态(高电平)
    IIC_SDA = 1;
    IIC_SCL = 1;  
}

/**
 * @brief  产生IIC起始信号
 * @param  None
 * @retval None
 * @note   起始条件: SCL高电平期间，SDA由高变低
 *         时序图:
 *         SCL: ‾‾‾‾‾‾‾‾\_____
 *         SDA: ‾‾‾‾\__________
 */
void IIC_Start(void)
{
    SDA_OUT();     // SDA设置为输出
    IIC_SDA = 1;   // SDA先拉高
    IIC_SCL = 1;   // SCL拉高
    delay_us(4);   // 建立时间
    IIC_SDA = 0;   // START: SCL高电平时，SDA由高变低
    delay_us(4);   // 保持时间
    IIC_SCL = 0;   // 钳住I2C总线，准备发送或接收数据
}

/**
 * @brief  产生IIC停止信号
 * @param  None
 * @retval None
 * @note   停止条件: SCL高电平期间，SDA由低变高
 *         时序图:
 *         SCL: _____/‾‾‾‾‾‾‾‾
 *         SDA: _________/‾‾‾‾
 */
void IIC_Stop(void)
{
    SDA_OUT();     // SDA设置为输出
    IIC_SCL = 0;   // SCL先拉低
    IIC_SDA = 0;   // SDA拉低
    delay_us(4);   // 建立时间
    IIC_SCL = 1;   // SCL拉高
    delay_us(4);   // 等待SCL稳定为高电平
    IIC_SDA = 1;   // STOP: SCL高电平时，SDA由低变高
    delay_us(4);   // 总线释放时间
}

/**
 * @brief  等待从机ACK应答信号
 * @param  None
 * @retval 0: 接收应答成功
 *         1: 接收应答失败(超时)
 * @note   主机释放SDA，从机在第9个时钟拉低SDA表示应答
 *         超时时间约250us
 */
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    
    SDA_IN();              // SDA设置为输入，释放SDA线
    IIC_SDA = 1;           // 使能内部上拉
    delay_us(1);
    IIC_SCL = 1;           // 时钟线拉高，读取应答
    delay_us(1);
    
    while(READ_SDA)        // 判断是否收到应答信号(低电平为有效应答)
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            IIC_Stop();    // 超时，发送停止信号
            return 1;      // 没有收到应答信号
        }
    }
    IIC_SCL = 0;           // 时钟线拉低，结束应答检测
    return 0;              // 收到应答
}

/**
 * @brief  产生ACK应答信号
 * @param  None
 * @retval None
 * @note   主机作为接收方时，拉低SDA表示继续接收
 */
void IIC_Ack(void)
{
    IIC_SCL = 0;           // 时钟线拉低
    SDA_OUT();             // SDA设置为输出
    IIC_SDA = 0;           // 发送应答信号(低电平)
    delay_us(2);
    IIC_SCL = 1;           // 时钟线拉高，让从机读取应答
    delay_us(2);
    IIC_SCL = 0;           // 时钟线拉低，结束应答
}

/**
 * @brief  产生NACK非应答信号
 * @param  None
 * @retval None
 * @note   主机作为接收方时，拉高SDA表示不再接收(通常用于最后一个字节)
 */
void IIC_NAck(void)
{
    IIC_SCL = 0;           // 时钟线拉低
    SDA_OUT();             // SDA设置为输出
    IIC_SDA = 1;           // 发送非应答信号(高电平)
    delay_us(2);
    IIC_SCL = 1;           // 时钟线拉高
    delay_us(2);
    IIC_SCL = 0;           // 时钟线拉低
}

/**
 * @brief  IIC发送一个字节
 * @param  txd: 要发送的字节数据
 * @retval None
 * @note   数据从高位(MSB)开始发送
 *         每个bit在SCL上升沿被从机采样
 */
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    
    SDA_OUT();             // SDA设置为输出
    IIC_SCL = 0;           // 拉低时钟开始数据传输
    
    for(t = 0; t < 8; t++)
    {
        // 发送数据位(从最高位开始)
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;         // 左移，准备下一位
        delay_us(2);       // 数据建立时间
        IIC_SCL = 1;       // SCL拉高，从机在此时采样SDA
        delay_us(2);       // 数据保持时间
        IIC_SCL = 0;       // SCL拉低，准备下一位
        delay_us(2);
    }
}

/**
 * @brief  IIC读取一个字节
 * @param  ack: 应答控制
 *         - 1: 发送ACK，表示继续接收
 *         - 0: 发送NACK，表示停止接收
 * @retval 读取到的字节数据
 * @note   数据从高位(MSB)开始接收
 *         在SCL高电平期间读取SDA
 */
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    
    SDA_IN();              // SDA设置为输入
    
    for(i = 0; i < 8; i++)
    {
        IIC_SCL = 0;       // 时钟线拉低
        delay_us(2);
        IIC_SCL = 1;       // 时钟线拉高
        receive <<= 1;     // 左移，准备接收下一位
        if(READ_SDA)       // SCL高电平期间，读取SDA上的数据
            receive++;
        delay_us(1);
    }
    
    // 发送应答或非应答
    if(!ack)
        IIC_NAck();        // 发送NACK
    else
        IIC_Ack();         // 发送ACK
    
    return receive;        // 返回读取到的数据
}
