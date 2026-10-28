#include "stm32f10x.h"                  
#include "Buzz.h"                  

// 蜂鸣器初始化函数

void Buzz_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置蜂鸣器引脚为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = BUZZ_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZ_PORT, &GPIO_InitStructure);
    
    // 初始化为高电平（蜂鸣器关闭）
    GPIO_SetBits(BUZZ_PORT, BUZZ_PIN);
}

// 关闭蜂鸣器函数
void Buzz_Off(void)
{
    GPIO_SetBits(BUZZ_PORT, BUZZ_PIN);
}

// 开启蜂鸣器函数（持续发声）
void Buzz_ON(void)
{
    GPIO_ResetBits(BUZZ_PORT, BUZZ_PIN);
}

