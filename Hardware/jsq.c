/**
 * @file    jsq.c
 * @brief   加湿器（JSQ）驱动程序
 * @details 实现加湿器的初始化、开关控制功能
 *          硬件连接：PA1引脚控制加湿器，PB9引脚预留按键
 */

#include "jsq.h"

/**
  * @brief  加湿器初始化函数
  * @param  无
  * @retval 无
  * @note   配置PA1为推挽输出（控制加湿器），PB9为上拉输入（预留按键）
  *         默认关闭加湿器
  */
void JSQ_Init(void)
{
	// 配置GPIO结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 1. 使能控制引脚和按键引脚时钟
	RCC_APB2PeriphClockCmd(JSQ_CLK | JSQ_KEY_CLK, ENABLE); 
	
	// 2. 初始化加湿器控制引脚（推挽输出）
	GPIO_InitStructure.GPIO_Pin = JSQ_GPIO_PIN;         // 加湿器控制引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 输出速度：50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出模式
	// 初始化GPIO
	GPIO_Init(JSQ_GPIO_PORT, &GPIO_InitStructure);

	// 3. 初始化加湿器按键引脚（上拉输入，按下为低）
	// 注意：当前主程序已统一使用Key.c的KEY1~KEY4扫描逻辑
	// 这里的JSQ_KEY仅做硬件预留，不影响主流程
	GPIO_InitStructure.GPIO_Pin = JSQ_KEY_PIN;          // 加湿器按键引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;       // 上拉输入模式，按键按下为低
	// 初始化GPIO
	GPIO_Init(JSQ_KEY_PORT, &GPIO_InitStructure);

	// 4. 默认关闭加湿器
	JSQ_OFF;
}
