#include "control.h"
#include "delay.h"

// 蜂鸣器初始化函数
void beep_init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 2. 配置PA8引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  // PA8
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // 速度50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

}

// LED初始化函数
void led_Init(void)
{
     GPIO_InitTypeDef  GPIO_InitStructure;

     // 使能GPIOA时钟
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

     // 配置PA5引脚
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                 // PA5
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 // 推挽输出
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 // IO速度50MHz
     GPIO_Init(GPIOA, &GPIO_InitStructure);					 // 初始化GPIOA5
     GPIO_SetBits(GPIOA, GPIO_Pin_5);						 // PA5置高

}








