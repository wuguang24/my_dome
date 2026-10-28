 #include "adc.h"
 #include "Delay.h"
	   
// 初始化ADC
// 本函数配置ADC1的通道0~3
// 默认使用PB0和PB1作为模拟输入通道																	   
void  Adc_Init(void)
{  
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	// 使能GPIOB和ADC1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
 
	// 配置ADC时钟为PCLK2的8分频（72M/8=9MHz），ADC最大时钟不能超过14MHz
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
                 
	// 配置PB0和PB1为模拟输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		// 模拟输入模式
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	// 复位ADC1，将ADC1的所有寄存器重置为默认值
	ADC_DeInit(ADC1);

	// 配置ADC1
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 	// ADC工作模式：独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE; 	// 禁止扫描模式，只使用单通道
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; 	// 禁止连续转换模式，使用单次转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 	// 转换由软件触发，不使用外部触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 	// ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1; 	// 规则通道序列长度为1
	ADC_Init(ADC1, &ADC_InitStructure); 	// 初始化ADC1

  
	// 启用ADC1
	ADC_Cmd(ADC1, ENABLE);
	
	// 复位校准寄存器
	ADC_ResetCalibration(ADC1);
	 
	// 等待复位校准完成
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	// 开始校准
	ADC_StartCalibration(ADC1);
 
	// 等待校准完成
	while(ADC_GetCalibrationStatus(ADC1));
}				 

// 读取ADC值
// 参数：ch - 通道号
// 返回值：ADC转换结果
u16 Get_Adc(u8 ch)   
{
   	// 配置指定ADC通道的采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_71Cycles5); 	// ADC1，指定通道，序列位置1，采样时间71.5个周期
  
	// 启动软件转换
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	 
	// 等待转换完成
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

	// 返回ADC转换结果
	return ADC_GetConversionValue(ADC1);
}

// 读取ADC平均值
// 参数：ch - 通道号，times - 采样次数
// 返回值：ADC转换结果的平均值
u16 Get_Adc_Average(u8 ch, u8 times)
{
	u32 temp_val = 0;
	u8 t;
	
	// 多次采样取平均值
	for(t = 0; t < times; t++)
	{
		temp_val += Get_Adc(ch);
		Delay_ms(1);  // 延时1ms，避免采样过于频繁
	}
	
	// 返回平均值
	return temp_val / times;
} 	 

