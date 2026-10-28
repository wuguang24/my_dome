#include "Actuator.h"

// 执行器驱动：风扇/补光灯/加湿器
// 引脚约定（可在此处集中修改映射）：
// - 风扇：PB14（低电平有效）
// - 补光：PB13（高电平有效）
// - 加湿：PB12（高电平有效）
// 注意：不同执行器模块的“有效电平”可能不同，本文件在 Set/Get 中已做统一封装。
#define ACT_GPIO_PORT GPIOB
#define ACT_FAN_PIN   GPIO_Pin_14
#define ACT_LIGHT_PIN GPIO_Pin_13
#define ACT_HUM_PIN   GPIO_Pin_12

static void Actuator_Write(GPIO_TypeDef* port, uint16_t pin, BitAction level)
{
    // GPIO 写封装：便于后续做反相/统一管理
    GPIO_WriteBit(port, pin, level);
}

void Actuator_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = ACT_FAN_PIN | ACT_LIGHT_PIN | ACT_HUM_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ACT_GPIO_PORT, &GPIO_InitStructure);

    // 上电默认关闭全部执行器，避免误动作
    Actuator_SetFan(0);
    Actuator_SetLight(0);
    Actuator_SetHumidifier(0);
}

void Actuator_SetFan(uint8_t on)
{
    // 风扇为低电平有效：
    // - on=1 -> 输出低电平
    // - on=0 -> 输出高电平
    if (on)
    {
        Actuator_Write(ACT_GPIO_PORT, ACT_FAN_PIN, Bit_RESET);
    }
    else
    {
        Actuator_Write(ACT_GPIO_PORT, ACT_FAN_PIN, Bit_SET);
    }
}

void Actuator_SetLight(uint8_t on)
{
    // 补光灯为高电平有效
    if (on)
    {
        Actuator_Write(ACT_GPIO_PORT, ACT_LIGHT_PIN, Bit_SET);
    }
    else
    {
        Actuator_Write(ACT_GPIO_PORT, ACT_LIGHT_PIN, Bit_RESET);
    }
}

void Actuator_SetHumidifier(uint8_t on)
{
    // 加湿器为高电平有效
    if (on)
    {
        Actuator_Write(ACT_GPIO_PORT, ACT_HUM_PIN, Bit_SET);
    }
    else
    {
        Actuator_Write(ACT_GPIO_PORT, ACT_HUM_PIN, Bit_RESET);
    }
}

uint8_t Actuator_GetFan(void)
{
    // 读取输出数据寄存器（ODR）判断当前输出状态，并转换为 on/off 语义
    return (GPIO_ReadOutputDataBit(ACT_GPIO_PORT, ACT_FAN_PIN) == Bit_RESET) ? 1 : 0;
}

uint8_t Actuator_GetLight(void)
{
    // 补光灯为高有效：ODR=1 表示开
    return (GPIO_ReadOutputDataBit(ACT_GPIO_PORT, ACT_LIGHT_PIN) == Bit_SET) ? 1 : 0;
}

uint8_t Actuator_GetHumidifier(void)
{
    // 加湿器为高有效：ODR=1 表示开
    return (GPIO_ReadOutputDataBit(ACT_GPIO_PORT, ACT_HUM_PIN) == Bit_SET) ? 1 : 0;
}
