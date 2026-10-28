#include "DHT11.h"
#include "stm32f10x.h"
#include "Delay.h"

// DHT11 温湿度传感器驱动（单总线协议）
// 通信要点（简述）：
// - MCU 先拉低数据线 >=18ms 作为起始信号
// - 释放后拉高 20~40us，然后切换为输入等待 DHT11 响应
// - DHT11 响应：80us 低 + 80us 高
// - 随后发送 40bit 数据：每一位以 50us 低电平开始，高电平宽度决定 0/1
//   - 0：高电平约 26~28us
//   - 1：高电平约 70us
// - 5 字节含义：湿度整数、湿度小数、温度整数、温度小数、校验和

// 更精确的微秒延时
// 注意：这是通过空循环粗略延时（与编译优化/主频相关），并非硬件定时器。
static void Delay_us(uint32_t us)
{
    us = us * (SystemCoreClock / 1000000) / 5;
    while(us--);
}

// DHT11初始化
// 将数据脚配置为上拉输入（空闲态为高），并等待传感器上电稳定。
void DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 初始化为上拉输入
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    Delay_ms(1000);  // DHT11 上电稳定时间（datasheet 建议）
}

// 读取 DHT11 一个位
// 原理：
// - DHT11 发送每一位时先拉低约 50us
// - 随后拉高：若在 35us 左右采样为高，则更可能是 1；为低则是 0
static uint8_t DHT11_Read_Bit(void)
{
    uint32_t timeout = 10000;
    
    // 等待起始低电平结束（低->高）
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == 0)
    {
        if(timeout-- == 0) return 0;
    }
    
    // 延时到采样点（经验值），避开高电平刚开始的边沿抖动
    Delay_us(35);
    
    // 读取当前电平
    uint8_t bit_val = GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN);
    
    // 等待高电平结束（高->低），为下一位做准备
    timeout = 10000;
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == 1)
    {
        if(timeout-- == 0) break;
    }
    
    return bit_val;
}

// 读取 DHT11 一个字节（高位先发）
static uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat = 0;
    for(i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

// 读取 DHT11 数据（一次完整事务 40bit）
// 返回值：SUCCESS/ERROR
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
    uint8_t buf[5] = {0};
    uint8_t i;
    uint32_t timeout;
    
    // 1) 起始信号：GPIO 切到推挽输出，由主机主动拉低
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
    
    // 主机拉低 >=18ms
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);
    Delay_ms(18);
    
    // 拉高 20~40us 后释放总线
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);
    Delay_us(30);
    
    // 2) 切换为输入模式（上拉），等待 DHT11 响应
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
    
    // DHT11 响应：先拉低约 80us
    timeout = 10000;
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == 1)
    {
        if(timeout-- == 0) return ERROR;
        Delay_us(1);
    }
    
    // 等待 80us 低电平结束
    timeout = 10000;
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == 0)
    {
        if(timeout-- == 0) return ERROR;
        Delay_us(1);
    }
    
    // 等待 80us 高电平结束（进入数据区）
    timeout = 10000;
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == 1)
    {
        if(timeout-- == 0) return ERROR;
        Delay_us(1);
    }
    
    // 3) 读取 40 位数据（5 字节）
    for(i = 0; i < 5; i++)
    {
        buf[i] = DHT11_Read_Byte();
    }
    
    // 4) 校验：最后 1 字节为前 4 字节之和的低 8 位
    if(buf[4] == (buf[0] + buf[1] + buf[2] + buf[3]))
    {
        *humi = buf[0];
        *temp = buf[2];
        return SUCCESS;
    }
    
    return ERROR;
}
