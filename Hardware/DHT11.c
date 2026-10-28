/**
 * @file    DHT11.c
 * @brief   DHT11温湿度传感器驱动程序
 * @details 实现DHT11的初始化、数据读取功能
 *          硬件连接：PB12引脚，单总线通信协议
 */

#include "dht11.h"

/* ==================== 全局变量定义 ==================== */
// 超时计数变量，用于检测通信超时
u8 Time = 0;

/**
  * @brief  DHT11初始化函数
  * @param  无
  * @retval 无
  * @note   使能GPIOB时钟，DHT11连接在PB12引脚
  */
void DHT11_Init(void)
{
	// 使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}

/**
  * @brief  DHT11模式切换函数
  * @param  mode 模式选择：OUT=输出模式，INT=输入模式
  * @retval 无
  * @note   DHT11使用单总线通信，需要在输入/输出模式间切换
  */
void DHT11_Mode(u8 mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if(mode)  // 输出模式
	{
		GPIO_InitStructure.GPIO_Pin = DHT11;           // DHT11引脚
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 输出速度50MHz
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // 推挽输出模式
	}
	else  // 输入模式
	{
		GPIO_InitStructure.GPIO_Pin = DHT11;               // DHT11引脚
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // 浮空输入模式
	}
	// 初始化GPIO
	GPIO_Init(DHT11_PROT, &GPIO_InitStructure);
}

/**
  * @brief  DHT11发送起始信号
  * @param  无
  * @retval 无
  * @note   主机拉低至少18ms，然后拉高40us，等待DHT11响应
  */
void DHT11_Sends_Start(void)
{
	// 切换为输出模式
	DHT11_Mode(OUT);
	
	// 拉低至少18ms，触发DHT11开始采集
	DHT11_Low;
	Delay_ms(18);  // 延时18ms
	
	// 拉高40us，等待DHT11响应
	DHT11_High;
	Delay_us(40);  // 延时40us
}

/**
  * @brief  DHT11等待响应信号
  * @param  无
  * @retval 无
  * @note   DHT11会先拉低80us，再拉高80us作为响应信号
  */
void DHT11_Sends_Response(void)
{
	// 切换为输入模式，等待DHT11响应
	DHT11_Mode(INT);
	
	// 等待DHT11拉低（低电平持续约80us）
	while(GPIO_ReadInputDataBit(DHT11_PROT,DHT11) == 0 && (Time < 100))
	{
		Time++;
		Delay_us(1);  // 每次延时1us，最长等待100us
	}
	Time = 0;  // 清零计时器
	
	// 等待DHT11拉高（高电平持续约80us）
	while(GPIO_ReadInputDataBit(DHT11_PROT,DHT11) == 1 && (Time < 100))
	{
		Time++;
		Delay_us(1);  // 每次延时1us，最长等待100us
	}
	Time = 0;  // 清零计时器
}

/**
  * @brief  DHT11读取一个字节数据
  * @param  无
  * @retval 读取到的字节数据
  * @note   每个位以50us低电平开始，高电平持续时间决定0或1
  *         - 高电平26-28us表示数据0
  *         - 高电平70us表示数据1
  */
u8 DHT11_Read_Byte(void)
{
	u8 data = 0;  // 存储读取的数据
	
	// 循环读取8位数据
	for(u8 i = 0; i < 8; i++)
	{
		// 切换为输入模式
		DHT11_Mode(INT);
		
		// 等待50us低电平过去
		while(GPIO_ReadInputDataBit(DHT11_PROT,DHT11) == 0 && (Time < 100))
		{
			Time++;
			Delay_us(1);
		}
		Time = 0;
		
		// 数据左移一位，为新数据腾出位置
		data <<= 1;
	
		// 延时40us后检测电平
		// 如果是数据0，高电平约26-28us，此时已变低
		// 如果是数据1，高电平约70us，此时仍为高
		Delay_us(40);
		
		// 检测电平判断数据位
		if(GPIO_ReadInputDataBit(DHT11_PROT, DHT11) == 1)
		{
			data |= 0x01;  // 如果是高电平，置1
		}
		
		// 等待高电平结束（数据1的高电平约70us）
		while(GPIO_ReadInputDataBit(DHT11_PROT,DHT11) == 1 && (Time < 100))
		{
			Time++;
			Delay_us(1);
		}
		Time = 0;
	}
	return data;  // 返回读取的字节
}

/**
  * @brief  DHT11读取温湿度数据
  * @param  humi 湿度数据指针（输出）
  * @param  temp 温度数据指针（输出）
  * @retval 无
  * @note   数据格式：湿度整数+湿度小数+温度整数+温度小数+校验和
  *         校验和=湿度整数+湿度小数+温度整数+温度小数
  */
void DHT11_Read_Data(u8 *humi, u8 *temp)
{
	// 数据缓冲区：5字节
	u8 DATA[5] = {0,0,0,0,0,};
	
	// 发送起始信号
	DHT11_Sends_Start();
	// 等待DHT11响应
	DHT11_Sends_Response();
	
	// 连续读取5个字节数据
	for(u8 i = 0; i < 5; i++)
	{
		DATA[i] = DHT11_Read_Byte();
	}
	
	// 延时1ms等待DHT11释放总线
	Delay_ms(1);
	
	// 校验数据：前4字节之和应等于第5字节
	if(DATA[0] + DATA[1] + DATA[2] + DATA[3] == DATA[4])
	{
		*humi = DATA[0];  // 湿度整数部分
		*temp = DATA[2];  // 温度整数部分
	}
	else
	{
		// 校验失败，清零数据
		for(u8 i = 0; i < 5; i++)
		{
			DATA[i] = 0;
		}
	}
}
