#include "stm32f10x.h"                  // 引入STM32F10x外设库头文件（GPIO/RCC等定义）

void LED_Init(void)                      // LED初始化函数（配置PC13/PC14为推挽输出）
{                                        // 函数体开始
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); // 使能GPIOC外设时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;    // 定义GPIO初始化结构体变量
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP; // 配置为推挽输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13 | GPIO_Pin_14; // 选择PC13与PC14两个引脚
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz; // 设置输出速度50MHz
	GPIO_Init(GPIOC,&GPIO_InitStructure);   // 初始化GPIOC指定引脚
	
	GPIO_SetBits(GPIOC,GPIO_Pin_13 | GPIO_Pin_14); // 置高PC13/PC14：低电平点亮 -> 默认熄灭
}                                        // 函数体结束
void LED_B_ON(void)                      // 蓝灯点亮函数（LED1：PC14，低电平点亮）
{                                        // 函数体开始
	GPIO_ResetBits(GPIOC, GPIO_Pin_14);     // 拉低PC14输出电平，点亮蓝灯
}                                        // 函数体结束

void LED_B_OFF(void)                     // 蓝灯熄灭函数（PC14，高电平熄灭）
{                                        // 函数体开始
	GPIO_SetBits(GPIOC, GPIO_Pin_14);       // 拉高PC14输出电平，熄灭蓝灯
}                                        // 函数体结束

void LED_B_Turn(void)                    // 蓝灯翻转函数（读取当前输出电平并翻转）
{                                        // 函数体开始
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_14) == 0) // 如果当前PC14输出为低电平（灯亮）
	{                                    // if分支开始
		GPIO_SetBits(GPIOC, GPIO_Pin_14);     // 置高PC14（灯灭）
	}                                    // if分支结束
	else                                   // 否则（当前PC14为高电平，灯灭）
	{                                    // else分支开始
		GPIO_ResetBits(GPIOC, GPIO_Pin_14);   // 置低PC14（灯亮）
	}                                    // else分支结束
}                                        // 函数体结束

void LED_R_ON(void)                      // 红灯点亮函数（LED2：PC13，低电平点亮）
{                                        // 函数体开始
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);     // 拉低PC13输出电平，点亮红灯
}                                        // 函数体结束

void LED_R_OFF(void)                     // 红灯熄灭函数（PC13，高电平熄灭）
{                                        // 函数体开始
	GPIO_SetBits(GPIOC, GPIO_Pin_13);       // 拉高PC13输出电平，熄灭红灯
}                                        // 函数体结束

void LED_R_Turn(void)                    // 红灯翻转函数（读取当前输出电平并翻转）
{                                        // 函数体开始
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0) // 如果当前PC13输出为低电平（灯亮）
	{                                    // if分支开始
		GPIO_SetBits(GPIOC, GPIO_Pin_13);     // 置高PC13（灯灭）
	}                                    // if分支结束
	else                                   // 否则（当前PC13为高电平，灯灭）
	{                                    // else分支开始
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);   // 置低PC13（灯亮）
	}                                    // else分支结束
}                                        // 函数体结束

//void LED_G_ON(void)                     // 预留：绿色LED点亮（当前未使用）
//{                                       // 预留：函数体开始
//	GPIO_ResetBits(GPIOC, GPIO_Pin_15);   // 预留：拉低PC15点亮
//}                                       // 预留：函数体结束
//                                        // 预留：空行占位
//void LED_G_OFF(void)                    // 预留：绿色LED熄灭（当前未使用）
//{                                       // 预留：函数体开始
//	GPIO_SetBits(GPIOC, GPIO_Pin_15);     // 预留：拉高PC15熄灭
//}                                       // 预留：函数体结束
//                                        // 预留：空行占位
//void LED_G_Turn(void)                   // 预留：绿色LED翻转（当前未使用）
//{                                       // 预留：函数体开始
//	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_15) == 0) // 预留：如果当前为低电平
//	{                                   // 预留：if分支开始
//		GPIO_SetBits(GPIOC, GPIO_Pin_15);   // 预留：置高
//	}                                   // 预留：if分支结束
//	else                                  // 预留：否则
//	{                                   // 预留：else分支开始
//		GPIO_ResetBits(GPIOC, GPIO_Pin_15); // 预留：置低
//	}                                   // 预留：else分支结束
//}                                       // 预留：函数体结束
