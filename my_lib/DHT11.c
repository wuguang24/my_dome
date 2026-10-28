#include "DHT11.h"
#include "stm32f10x.h"
#include "Delay.h"
#include <stddef.h>

// DHT11传感器初始化函数
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
    
    Delay_ms(1000);  // DHT11上电稳定时间
}

// 读取DHT11一个位
// 返回值：读取到的位值（0或1）
static uint8_t DHT11_Read_Bit(void)
{
    uint32_t timeout = 10000;
    
    // 等待50us低电平
    while(GPIO_ReadInputDataBit(GPIOA, DHT11_PIN) == 0)
    {
        if(timeout-- == 0) return 0;
    }
    
    // 延时30-50us判断位值
    Delay_us(35);
    
    // 读取当前电平
    uint8_t bit_val = GPIO_ReadInputDataBit(GPIOA, DHT11_PIN);
    
    // 等待高电平结束
    timeout = 10000;
    while(GPIO_ReadInputDataBit(GPIOA, DHT11_PIN) == 1)
    {
        if(timeout-- == 0) break;
    }
    
    return bit_val;
}

// 读取DHT11一个字节
// 返回值：读取到的字节数据
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

// 读取DHT11温湿度数据
// 参数：temp - 温度值指针，humi - 湿度值指针
// 返回值：SUCCESS - 读取成功，ERROR - 读取失败
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
    uint8_t try_cnt = 0;

    if(temp == NULL || humi == NULL)
        return ERROR;

    for(try_cnt = 0; try_cnt < 3; try_cnt++)  // 最多尝试3次
    {
        uint8_t buf[5] = {0};  // 存储读取到的数据
        uint8_t i;
        uint32_t timeout;

        // 设置为开漏输出模式
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        // 主机拉低18ms，发送开始信号
        GPIO_ResetBits(GPIOA, DHT11_PIN);
        Delay_ms(18);

        // 拉高20-40us，准备接收DHT11的响应
        GPIO_SetBits(GPIOA, DHT11_PIN);
        Delay_us(30);

        // 设置为输入模式（上拉）
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        // DHT11时序敏感：从此处开始尽量避免中断打断
        __disable_irq();

        // 等待DHT11的80us低电平响应
        timeout = 10000;
        while(GPIO_ReadInputDataBit(GPIOA, DHT11_PIN) == 1)
        {
            if(timeout-- == 0) { __enable_irq(); goto TRY_NEXT; }
            Delay_us(1);
        }

        // 等待80us低电平结束
        timeout = 10000;
        while(GPIO_ReadInputDataBit(GPIOA, DHT11_PIN) == 0)
        {
            if(timeout-- == 0) { __enable_irq(); goto TRY_NEXT; }
            Delay_us(1);
        }

        // 等待80us高电平结束
        timeout = 10000;
        while(GPIO_ReadInputDataBit(GPIOA, DHT11_PIN) == 1)
        {
            if(timeout-- == 0) { __enable_irq(); goto TRY_NEXT; }
            Delay_us(1);
        }

        // 读取40位数据（5字节）
        for(i = 0; i < 5; i++)
        {
            buf[i] = DHT11_Read_Byte();
        }

        __enable_irq();  // 重新启用中断

        // 校验数据（前4字节之和等于第5字节）
        if(buf[4] == (uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]))
        {
            *humi = buf[0];  // 湿度值
            *temp = buf[2];  // 温度值
            return SUCCESS;
        }

TRY_NEXT:
        Delay_ms(20);  // 等待20ms后重试
    }

    return ERROR;  // 3次尝试都失败
}
