#ifndef __ADC_H
#define __ADC_H	
#include "stm32f10x.h"

// 函数声明

// ADC初始化函数
void Adc_Init(void);
// 读取单个ADC值
// 参数：ch - ADC通道号
// 返回值：ADC转换结果
u16  Get_Adc(u8 ch); 
// 读取ADC平均值
// 参数：ch - ADC通道号，times - 采样次数
// 返回值：ADC转换结果的平均值
u16 Get_Adc_Average(u8 ch,u8 times);
 
#endif 
