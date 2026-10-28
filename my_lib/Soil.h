#ifndef __Soil_H
#define __Soil_H
#include "stm32f10x.h"                  // Device header

// 土壤湿度传感器引脚定义
#define		Soil_AO_GPIO_CLK			RCC_APB2Periph_GPIOA  // 土壤传感器AO引脚时钟
#define 	Soil_AO_GPIO_PORT		    GPIOA                // 土壤传感器AO引脚端口
#define 	Soil_AO_GPIO_PIN			GPIO_Pin_0            // 土壤传感器AO引脚
#define     Soil_ADC_CHANNEL            ADC_Channel_0       // ADC通道定义

// 函数声明
// 土壤湿度传感器初始化函数
void Soil_Init(void);
// 读取土壤湿度传感器ADC值
// 参数：ADC_Channel - ADC通道号
// 返回值：ADC转换结果
uint16_t Soil_AD_GetValue(uint8_t ADC_Channel);
// 读取ADC值
// 参数：ADC_Channel - ADC通道号
// 返回值：ADC转换结果
uint16_t AD_GetValue(uint8_t ADC_Channel);
// 土壤湿度值转换函数
// 返回值：土壤湿度百分比（0-100%）
float Soil_Value_Conversion(void);

#endif
