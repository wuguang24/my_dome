#include "bump.h"
#include "stm32f10x.h"
/*****************水泵模块源文件******************
 * 文件			: 5V水泵模块c文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.22
 * MCU			: STM32F103C8T6
 * 接口			: 通用IO口									
 * 作者			: 野火 
 * 功能			: 控制水泵的开关
**********************BEGIN***********************/

// 水泵初始化函数
void BUMP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 使能水泵GPIO时钟
	RCC_APB2PeriphClockCmd(BUMP_CLK, ENABLE);
	
	// 配置水泵GPIO引脚
	GPIO_InitStructure.GPIO_Pin = BUMP_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
	GPIO_Init(BUMP_GPIO_PROT, &GPIO_InitStructure);

	// 初始化关闭水泵
	BUMP_OFF;
}

