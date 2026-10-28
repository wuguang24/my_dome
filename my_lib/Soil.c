#include "Soil.h"              
#include "delay.h"      

// 土壤湿度传感器初始化函数
void Soil_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   ADC_InitTypeDef ADC_InitStructure;
	
	// 使能ADC1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	// 使能土壤传感器AO引脚时钟
	RCC_APB2PeriphClockCmd(Soil_AO_GPIO_CLK, ENABLE);
	
	// 配置ADC时钟为PCLK2的6分频（12MHz）
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	// 配置土壤传感器AO引脚为模拟输入模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = Soil_AO_GPIO_PIN ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Soil_AO_GPIO_PORT, &GPIO_InitStructure);

	// 配置ADC1
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // 独立模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  // 数据右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // 无外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // 单次转换模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;  // 非扫描模式
	ADC_InitStructure.ADC_NbrOfChannel = 1;  // 1个通道
	ADC_Init(ADC1, &ADC_InitStructure);
	
	// 启用ADC1
	ADC_Cmd(ADC1, ENABLE);
	
	// 校准ADC1
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

// 读取土壤湿度传感器ADC值
// 参数：ADC_Channel - ADC通道号
// 返回值：ADC转换结果
uint16_t Soil_AD_GetValue(uint8_t ADC_Channel)
{
	// 配置ADC1规则通道
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);
	// 启动软件转换
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	// 等待转换完成
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	// 返回转换结果
	return ADC_GetConversionValue(ADC1);
}

// 读取ADC值（未使用，预留函数）
// 参数：ADC_Channel - ADC通道号
// 返回值：ADC转换结果
uint16_t AD_GetValue(uint8_t ADC_Channel)
{
	// 配置ADC1规则通道
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);
	// 启动软件转换
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	// 等待转换完成
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	// 返回转换结果
	return ADC_GetConversionValue(ADC1);
}

/**************土壤湿度值转换函数***************/
// 返回值：土壤湿度百分比（0-100%）
float Soil_Value_Conversion(void)
{
    u32 ad=0;  // 累加ADC值
    u8 i;  // 循环变量
    float Soil_value=0.0;  // 土壤湿度值
    ad=0;
    
    // 读取50次ADC值取平均值，提高准确性	
    for(i=0;i<50;i++)
    {
        ad=ad+Soil_AD_GetValue(Soil_ADC_CHANNEL);  // 读取一次ADC转换结果	
    }
    ad=ad/50;  // 计算平均值
    
    // AD转换：将0-4095的ADC值转换为0-100%的湿度值
    // 注意：土壤湿度传感器的输出与湿度成反比，湿度越大，ADC值越小
    Soil_value = 100 - (float)ad / 4096 * 100;
    
    return Soil_value;
}

