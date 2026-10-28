#include "led_module.h"


// LED模块源文件                   


// LED模块初始化函数
void LED_MODULE_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 使能LED模块GPIO时钟
	RCC_APB2PeriphClockCmd(LED_MODULE_CLK, ENABLE);
	
	// 配置LED模块GPIO引脚
	GPIO_InitStructure.GPIO_Pin = LED_MODULE_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
	GPIO_Init(LED_MODULE_GPIO_PORT, &GPIO_InitStructure);

	// 初始化关闭LED模块
	LED_MODULE_OFF;
}

