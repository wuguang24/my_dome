/**
 * @file    LDR.c
 * @brief   光敏电阻（LDR）驱动程序
 * @details 实现光敏电阻的初始化、ADC读取、光照强度计算功能
 *          硬件连接：PA0引脚，ADC通道0
 */

#include "ldr.h"

/**
  * @brief  LDR初始化函数
  * @param  无
  * @retval 无
  * @note   配置PA0为模拟输入模式，用于ADC采集光照强度
  */
void LDR_Init(void)
{
	// 使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// 配置GPIO结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	// 模拟输入模式：用于ADC采集
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	// LDR引脚：PA0
	GPIO_InitStructure.GPIO_Pin = LDR;
	// 输出速度（模拟输入模式下无效）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// 初始化GPIO
	GPIO_Init(LDR_PROT, &GPIO_InitStructure);
}

/**
  * @brief  LDR ADC读取函数
  * @param  无
  * @retval ADC采样值（0-4095）
  * @note   读取ADC通道0的值，采样周期55.5个ADC时钟周期
  */
uint16_t LDR_ADC_Read(void)
{
	// 读取指定ADC的规则通道值
	return ADC_GetValue(ADC_Channel_0, ADC_SampleTime_55Cycles5);
}

/**
  * @brief  LDR多次采样取平均值函数
  * @param  无
  * @retval 平均ADC值（0-4095）
  * @note   采样LDR_READ_TIMES次后取平均值，提高数据稳定性
  */
uint16_t LDR_Average_Data(void)
{
	uint32_t tempData = 0;  // 累加变量
	
	// 循环采样多次
	for (uint8_t i = 0; i < LDR_READ_TIMES; i++)
	{
		tempData += LDR_ADC_Read();  // 累加ADC值
		Delay_ms(5);  // 每次采样间隔5ms
	}

	// 计算平均值
	tempData /= LDR_READ_TIMES;
	return (uint16_t)tempData;
}

/**
  * @brief  LDR光照强度计算函数
  * @param  Lux 光照强度指针（输出），单位：勒克斯(lux)
  * @retval 无
  * @note   根据ADC值计算光照强度
  *         计算公式：Lux = 40000 * R^(-0.6021)
  *         其中R为光敏电阻阻值，通过分压电路计算得到
  */
void LDR_LuxData(uint16_t *Lux)
{
	float voltage = 0;  // 电压值
	float R = 0;        // 光敏电阻阻值
	
	// 获取ADC平均值
	voltage = LDR_Average_Data();
	// 将ADC值转换为电压值（0-4095对应0-3.3V）
	voltage = voltage / 4096 * 3.3f;
	
	// 根据分压电路计算光敏电阻阻值
	// 假设分压电阻为10kΩ，Vout = 3.3 * R / (R + 10k)
	// R = Vout * 10k / (3.3 - Vout)
	R = voltage / (3.3f - voltage) * 10000;
	
	// 根据光敏电阻特性计算光照强度
	// 公式来源于光敏电阻数据手册的拟合曲线
	*Lux = 40000 * pow(R, -0.6021);
	
	// 限制最大值为999
	if (*Lux > 999)
	{
		*Lux = 999;
	}
}
