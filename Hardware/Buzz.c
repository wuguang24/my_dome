#include "stm32f10x.h"                  
#include "Buzz.h"                  

// 蜂鸣器驱动
// 说明：本工程蜂鸣器为“低电平触发”（GPIO 输出 0 时响，输出 1 时停）。
// 具体引脚由 Buzz.h 中的 BUZZ_PORT / BUZZ_PIN 决定。

// 蜂鸣器初始化
// 将蜂鸣器 GPIO 配置为推挽输出，并设置默认关闭状态。

void Buzz_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置蜂鸣器引脚为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = BUZZ_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZ_PORT, &GPIO_InitStructure);
    
    // 初始化为高电平（蜂鸣器关闭，低电平触发型）
    GPIO_SetBits(BUZZ_PORT, BUZZ_PIN);
}

// 关闭蜂鸣器（输出高电平）
void Buzz_Off(void)
{
    GPIO_SetBits(BUZZ_PORT, BUZZ_PIN);
}

// 蜂鸣器响（输出低电平）
// 备注：若需要间歇鸣叫，可以在业务层用定时器/计数翻转调用本函数与 Buzz_Off()。
void Buzz_ON(void)
{
    GPIO_ResetBits(BUZZ_PORT, BUZZ_PIN);
}

