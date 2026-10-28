/**
 * @file    WaterLevel.c
 * @brief   水位传感器驱动程序
 * @details 实现水位传感器的初始化、水位检测功能
 *          硬件连接：PA4引脚
 *          传感器原理：通过导电检测液位，有水=低电平，无水=高电平
 */

#include "WaterLevel.h"

/**
  * @brief  水位传感器初始化函数
  * @param  无
  * @retval 无
  * @note   配置PA4为上拉输入模式
  *         传感器OUT：无水=高电平，有水=低电平（低有效）
  */
void WaterLevel_Init(void)
{
	// 配置GPIO结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	// 使能水位传感器GPIO时钟
	RCC_APB2PeriphClockCmd(WATERLEVEL_CLK, ENABLE);

	// 配置为上拉输入模式：
	// 传感器OUT：无水=高电平，有水=低电平（低有效）
	GPIO_InitStructure.GPIO_Pin = WATERLEVEL_PIN;      // 水位传感器引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 输出速度（输入模式下无效）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      // 上拉输入模式
	// 初始化GPIO
	GPIO_Init(WATERLEVEL_PORT, &GPIO_InitStructure);
}

/**
  * @brief  水位检测函数
  * @param  无
  * @retval 1=检测到水（有水），0=未检测到水（缺水）
  * @note   低电平表示检测到水，高电平表示缺水
  */
uint8_t WaterLevel_IsWaterDetected(void)
{
	// 低电平表示检测到水
	return (GPIO_ReadInputDataBit(WATERLEVEL_PORT, WATERLEVEL_PIN) == Bit_RESET) ? 1 : 0;
}
