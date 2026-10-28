#include "stm32f10x.h"
#include "Delay.h"

// 软件 I2C（模拟 I2C / Bit-Bang）
// 引脚约定：
// - SCL：PB6
// - SDA：PB7
// GPIO 配置为开漏输出（GPIO_Mode_Out_OD），并把引脚拉高以“释放总线”。
// 注意：I2C 总线需要上拉电阻（外部或内部弱上拉），否则 SDA/SCL 不能正常回到高电平。
// 本实现通过 Delay_us 控制时序，延时越大总线越慢。

/*引脚配置层*/

/**
  * @brief  I2C写SCL引脚电平
  * @param  BitValue 电平值（0或1）
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
    // 写 SCL：I2C 时钟线
    GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)BitValue);  // PB6
    Delay_us(10);
}

/**
  * @brief  I2C写SDA引脚电平
  * @param  BitValue 电平值
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
    // 写 SDA：I2C 数据线
    GPIO_WriteBit(GPIOB, GPIO_Pin_7, (BitAction)BitValue);  // PB7
    Delay_us(10);
}

/**
  * @brief  I2C读SDA引脚电平
  * @retval SDA引脚电平（0或1）
  */
uint8_t MyI2C_R_SDA(void)
{
    uint8_t BitValue;
    // 读 SDA：注意在读之前应先把 SDA 置 1（开漏释放），让从机驱动电平
    BitValue = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);     // PB7
    Delay_us(10);
    return BitValue;
}

/**
  * @brief  I2C初始化
  */
void MyI2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;   // PB6和PB7
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 设置为高电平（开漏输出下相当于“释放总线”），启用内部上拉/外部上拉生效
    GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
}

/*协议层*/

/**
  * @brief  I2C起始信号
  */
void MyI2C_Start(void)
{
    // 起始条件：SCL=1 时 SDA 从 1 跳变到 0
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(0);
}

/**
  * @brief  I2C停止信号
  */
void MyI2C_Stop(void)
{
    // 停止条件：SCL=1 时 SDA 从 0 跳变到 1
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的字节
  */
void MyI2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        // 主机在 SCL 低电平期间更新 SDA，在 SCL 上升沿后数据被采样
        MyI2C_W_SDA(Byte & (0x80 >> i));
        MyI2C_W_SCL(1);
        MyI2C_W_SCL(0);
    }
}

/**
  * @brief  I2C接收一个字节
  * @retval 接收到的字节
  */
uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    // 释放 SDA 让从机输出
    MyI2C_W_SDA(1);
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SCL(1);
        if (MyI2C_R_SDA() == 1)
        {
            Byte |= (0x80 >> i);
        }
        MyI2C_W_SCL(0);
    }
    return Byte;
}

/**
  * @brief  I2C发送应答位
  * @param  AckBit 应答位（0：应答，1：非应答）
  */
void MyI2C_SendAck(uint8_t AckBit)
{
    // ACK/NACK：主机在第 9 个时钟周期驱动 SDA
    MyI2C_W_SDA(AckBit);
    MyI2C_W_SCL(1);
    MyI2C_W_SCL(0);
}

/**
  * @brief  I2C接收应答位
  * @retval 应答位（0：应答，1：非应答）
  */
uint8_t MyI2C_ReceiveAck(void)
{
    uint8_t AckBit;
    // 释放 SDA，由从机在第 9 个时钟周期输出 ACK(0) / NACK(1)
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    AckBit = MyI2C_R_SDA();
    MyI2C_W_SCL(0);
    return AckBit;
}
