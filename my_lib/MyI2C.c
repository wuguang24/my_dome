#include "stm32f10x.h"
#include "Delay.h"

/*引脚配置层*/

/**
  * @brief  I2C写SCL引脚电平
  * @param  BitValue 电平值（0或1）
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_5, (BitAction)BitValue);  // PB5
    Delay_us(10);
}

/**
  * @brief  I2C写SDA引脚电平
  * @param  BitValue 电平值
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)BitValue);  // PB6
    Delay_us(10);
}

/**
  * @brief  I2C读SDA引脚电平
  * @retval SDA引脚电平（0或1）
  */
uint8_t MyI2C_R_SDA(void)
{
    uint8_t BitValue;
    BitValue = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);     // PB6
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
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;   // PB5和PB6
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 设置为高电平，启用内部上拉
    GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_6);
}

/*协议层*/

/**
  * @brief  I2C起始信号
  */
void MyI2C_Start(void)
{
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
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    AckBit = MyI2C_R_SDA();
    MyI2C_W_SCL(0);
    return AckBit;
}
