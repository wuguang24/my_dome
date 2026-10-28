#include "fan.h"

/*****************风扇模块源文件******************
 * 文件			: 5V风扇模块c文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.21
 * MCU			: STM32F103C8T6
 * 接口			: 通用IO口									
 * 作者			: 野火 
 * 功能			: 控制风扇的开关
**********************BEGIN***********************/

// 风扇初始化函数
void FAN_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 使能风扇GPIO时钟
	RCC_APB2PeriphClockCmd(FAN_CLK, ENABLE);
	
	// 配置风扇GPIO引脚
	GPIO_InitStructure.GPIO_Pin = FAN_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
	GPIO_Init(FAN_GPIO_PROT, &GPIO_InitStructure);

	// 初始化关闭风扇
	FAN_OFF;
}

