/*
 * 文件名称: ctiic.c
 * 模块功能: 电容触摸屏IIC驱动实现
 * 
 * 功能说明:
 *   - 使用GPIO模拟IIC总线时序
 *   - 专用于电容触摸屏芯片(GT1151/GT9147/FT5206等)
 *   - 实现IIC协议的起始、停止、发送、接收、应答等操作
 * 
 * 硬件连接:
 *   - SCL: PB15 (时钟线)
 *   - SDA: PB12 (数据线)
 * 
 * 时序特点:
 *   - 相比标准IIC，延时稍长以适配触摸IC
 *   - 使用delay_us(30)等较长延时确保稳定
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 * 日期: 2025-12-12
 */

#include "ctiic.h"
#include "delay.h"

/**
 * @brief  IIC速度调节延时
 * @param  None
 * @retval None
 */
void CT_Delay(void)
{
    delay_us(2);
}

/**
 * @brief  电容触摸屏IIC接口初始化
 * @param  None
 * @retval None
 * @note   配置PB15(SCL)和PB12(SDA)为推挽输出，上拉，高速
 */
void CT_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();   // 使能GPIOB时钟

    // PB15配置为输出(SCL)
    GPIO_Initure.Pin = GPIO_PIN_15;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;          // 上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;     // 高速
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
    
    // PB12配置为输出(SDA)
    GPIO_Initure.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
}

/**
 * @brief  产生IIC起始信号
 * @param  None
 * @retval None
 * @note   起始条件: SCL高电平期间，SDA由高变低
 */
void CT_IIC_Start(void)
{
    CT_SDA_OUT();      // SDA设置为输出
    CT_IIC_SDA = 1;
    CT_IIC_SCL = 1;
    delay_us(30);      // 建立时间(触摸IC需要较长延时)
    CT_IIC_SDA = 0;    // START: SCL高电平时，SDA由高变低
    CT_Delay();
    CT_IIC_SCL = 0;    // 钳住I2C总线，准备发送或接收数据
}

/**
 * @brief  产生IIC停止信号
 * @param  None
 * @retval None
 * @note   停止条件: SCL高电平期间，SDA由低变高
 */
void CT_IIC_Stop(void)
{
    CT_SDA_OUT();      // SDA设置为输出
    CT_IIC_SCL = 1;
    delay_us(30);
    CT_IIC_SDA = 0;    // STOP: SCL高电平时，SDA由低变高
    CT_Delay();
    CT_IIC_SDA = 1;    // 发送I2C总线结束信号
}

/**
 * @brief  等待从机ACK应答信号
 * @param  None
 * @retval 0: 接收应答成功
 *         1: 接收应答失败(超时)
 */
u8 CT_IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    
    CT_SDA_IN();           // SDA设置为输入
    CT_IIC_SDA = 1;        // 释放SDA线
    CT_IIC_SCL = 1;        // 时钟线拉高
    CT_Delay();
    
    while(CT_READ_SDA)     // 判断是否收到应答信号(低电平为有效应答)
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            CT_IIC_Stop();
            return 1;      // 没有收到应答信号
        }
        CT_Delay();
    }
    CT_IIC_SCL = 0;        // 时钟线拉低
    return 0;
}

/**
 * @brief  产生ACK应答信号
 * @param  None
 * @retval None
 */
void CT_IIC_Ack(void)
{
    CT_IIC_SCL = 0;
    CT_SDA_OUT();          // SDA设置为输出
    CT_Delay();
    CT_IIC_SDA = 0;        // 发送应答信号
    CT_Delay();
    CT_IIC_SCL = 1;
    CT_Delay();
    CT_IIC_SCL = 0;
}

/**
 * @brief  产生NACK非应答信号
 * @param  None
 * @retval None
 */
void CT_IIC_NAck(void)
{
    CT_IIC_SCL = 0;
    CT_SDA_OUT();          // SDA设置为输出
    CT_Delay();
    CT_IIC_SDA = 1;        // 发送非应答信号
    CT_Delay();
    CT_IIC_SCL = 1;
    CT_Delay();
    CT_IIC_SCL = 0;
}

/**
 * @brief  IIC发送一个字节
 * @param  txd: 要发送的字节数据
 * @retval None
 * @note   数据从高位(MSB)开始发送
 */
void CT_IIC_Send_Byte(u8 txd)
{
    u8 t;
    
    CT_SDA_OUT();          // SDA设置为输出
    CT_IIC_SCL = 0;        // 拉低时钟开始数据传输
    CT_Delay();
    
    for(t = 0; t < 8; t++)
    {
        CT_IIC_SDA = (txd & 0x80) >> 7;  // 发送最高位
        txd <<= 1;
        CT_Delay();
        CT_IIC_SCL = 1;    // SCL拉高，从机采样
        CT_Delay();
        CT_IIC_SCL = 0;
        CT_Delay();
    }
}

/**
 * @brief  IIC读取一个字节
 * @param  ack: 应答控制 (1=发送ACK, 0=发送NACK)
 * @retval 读取到的字节数据
 */
u8 CT_IIC_Read_Byte(unsigned char ack)
{
    u8 i, receive = 0;
    
    CT_SDA_IN();           // SDA设置为输入
    delay_us(30);          // 等待SDA稳定
    
    for(i = 0; i < 8; i++)
    {
        CT_IIC_SCL = 0;
        CT_Delay();
        CT_IIC_SCL = 1;
        receive <<= 1;
        CT_Delay();
        if(CT_READ_SDA)    // SCL高电平期间读取SDA
            receive++;
        CT_Delay();
    }
    
    if(!ack)
        CT_IIC_NAck();     // 发送NACK
    else
        CT_IIC_Ack();      // 发送ACK
    
    return receive;
}
