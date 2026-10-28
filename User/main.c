/**
 * @file    main.c
 * @brief   智能香薰灯主程序
 * @details 实现功能：温湿度检测、光照检测、LED亮度控制、加湿器控制、
 *          OLED显示、按键控制、语音控制、缺水报警等
 */

/* ==================== 头文件包含 ==================== */
#include "stm32f10x.h"                  // STM32F10x标准外设库头文件
#include "ldr.h"                        // 光敏电阻驱动头文件
#include "oled.h"                       // OLED显示屏驱动头文件
#include "dht11.h"                      // DHT11温湿度传感器驱动头文件
#include "pwm.h"                        // PWM脉宽调制驱动头文件
#include "led.h"                        // LED灯驱动头文件
#include "key.h"                        // 按键驱动头文件
#include "sensormodules.h"              // 传感器模块数据结构头文件
#include "flash.h"                      // Flash存储驱动头文件
#include "iwdg.h"                       // 独立看门狗驱动头文件
#include "usart3.h"                     // 串口3驱动头文件（用于语音模块通信）
#include "usart.h"                      // 串口1驱动头文件（用于调试打印）
#include "jsq.h"                        // 加湿器驱动头文件
#include "WaterLevel.h"                 // 水位检测驱动头文件

/* ==================== 外部函数声明 ==================== */
// TIM2 2ms中断中维护的毫秒计数（在 System/TIM2.c 实现），用于主循环做稳定的软件定时
uint32_t TIM2_GetMs(void);

/* ==================== 全局变量定义 ==================== */
// 加湿器状态变量：0=关闭，1=开启
uint8_t jsq_state = 0;
// 加湿器按键消抖锁存标志
uint8_t jsq_key_lock = 0;

/* ==================== 按键键值宏定义 ==================== */
#define KEY_Long1	11    // KEY1长按键值
#define KEY_1	1         // KEY1短按键值
#define KEY_2	2         // KEY2短按键值
#define KEY_3	3         // KEY3短按键值
#define KEY_4	4         // KEY4短按键值

/* ==================== Flash存储地址定义 ==================== */
// Flash存储起始地址，用于保存阈值等参数
#define FLASH_START_ADDR	0x0801f000

/* ==================== 系统状态全局变量 ==================== */
// 系统模式：0=手动模式，1=自动模式
uint8_t systemModel = 0;
// 菜单页面：1=显示页，2=设置页
uint8_t menu = 1;
// 时间变量（时、分、秒）
uint8_t hour,minute,second = 0;
// 传感器数据结构体实例
SensorModules sensorData;
// 传感器阈值结构体实例
SensorThresholdValue Sensorthreshold;
// OLED刷新计时变量（毫秒）
uint32_t oled_tick = 0;
// OLED强制刷新标志位：1=需要刷新，0=不需要刷新
uint8_t oled_refresh_flag = 0;
// KEY2按下次数计数器，用于控制亮度循环（1->2->3->0循环）
uint8_t key2_press_count = 0;

/* ==================== OLED页面重绘控制变量 ==================== */
// 用于减少全屏重绘导致的"闪烁"
// 上一次的菜单页面值
static uint8_t last_menu = 0;
// 静态内容是否已绘制标志：1=已绘制，0=未绘制
static uint8_t display_static_drawn = 0;

/* ==================== OLED动态区缓存变量 ==================== */
// 避免每200ms重绘中文导致"闪屏"
// 上一次的系统模式值（初始化为0xFF表示首次必定刷新）
static uint8_t last_systemModel = 0xFF;
// 上一次的加湿器状态值（初始化为0xFF表示首次必定刷新）
static uint8_t last_jsq_state = 0xFF;

/* ==================== 加湿器控制相关变量 ==================== */
// jsq_state：逻辑开关状态（由KEY3/水位保护/上位机等决定）
// jsq_output_state：实际输出状态（用于"间隔模式"下的开/关节拍）
// 加湿器间隔模式：0=常开，1=一档，2=二档，3=三档
static uint8_t jsq_interval_mode = 0;
// 加湿器实际输出状态：0=关闭，1=开启
static uint8_t jsq_output_state = 0;
// 加湿器间隔计时变量（毫秒）
static uint32_t jsq_interval_tick = 0;

/* ==================== 缺水报警（蜂鸣器）相关变量 ==================== */
// 蜂鸣器周期计时变量（毫秒）
static uint32_t buzzer_tick = 0;
// 蜂鸣器开启计时变量（毫秒）
static uint32_t buzzer_on_tick = 0;

/* ==================== 水位软去抖相关变量 ==================== */
// 防止水位输入抖动导致误报警/蜂鸣器一直响
// 水位稳定状态：1=有水，0=缺水（与sensorData.waterLevel一致）
static uint8_t water_level_stable = 1;
// 水位待确认状态
static uint8_t water_level_pending = 1;
// 水位去抖计数器
static uint8_t water_level_cnt = 0;

/* ==================== USART1调试打印相关变量 ==================== */
// 调试打印计时变量（毫秒）
static uint32_t debug_tick = 0;
// 上一次缺水状态标志：用于检测缺水边沿事件
static uint8_t last_water_low = 0;

/* ==================== 菜单页面枚举定义 ==================== */
enum 
{
	display_page = 1,       // 显示页面：显示传感器数据和系统状态
	settingsPage,           // 设置页面：设置阈值参数
	timeSettingsPage        // 时间设置页面（预留）
}MenuPages;

/**
  * @brief  OLED主页面固定内容显示
  * @param  无
  * @retval 无
  * @note   显示"智能香薰灯"标题和各传感器标签
  */
void OLED_Menu(void)
{
	// 显示"智能香薰灯"标题（中文编码30-34）
	OLED_ShowChinese(1, 3, 30);  // 显示"智"
	OLED_ShowChinese(1, 4, 31);  // 显示"能"
	OLED_ShowChinese(1, 5, 32);  // 显示"香"
	OLED_ShowChinese(1, 6, 33);  // 显示"薰"
	OLED_ShowChinese(1, 7, 34);  // 显示"灯"
	
	// 显示"温度："标签（中文编码0-1）
	OLED_ShowChinese(2,1, 0);    // 显示"温"
	OLED_ShowChinese(2,2, 1);    // 显示"度"
	OLED_ShowChar(2, 5, ':');    // 显示冒号
	
	// 显示"湿度："标签（中文编码2-1）
	OLED_ShowChinese(2,5, 2);    // 显示"湿"
	OLED_ShowChinese(2,6, 1);    // 显示"度"
	OLED_ShowChar(2, 13, ':');   // 显示冒号
	
	// 显示"光强："标签（中文编码4-5）
	OLED_ShowChinese(3,1, 4);    // 显示"光"
	OLED_ShowChinese(3,2, 5);    // 显示"强"
	OLED_ShowChar(3, 5, ':');    // 显示冒号
	OLED_ShowChar(3, 13, ' ');   // 显示空格
		
	// 显示"亮度："标签（中文编码6-1）
	OLED_ShowChinese(4,1, 6);    // 显示"亮"
	OLED_ShowChinese(4,2, 1);    // 显示"度"
	OLED_ShowChar(4, 5, ':');    // 显示冒号
	
	// 显示"香薰："标签（中文编码32-33）
	OLED_ShowChinese(3, 5, 32);  // 显示"香"
	OLED_ShowChinese(3, 6, 33);  // 显示"薰"
	OLED_ShowChar(3, 13, ':');   // 显示冒号
}

/**
  * @brief  OLED主页面传感器数据显示
  * @param  无
  * @retval 无
  * @note   显示温度、湿度、光强、亮度、模式、加湿器状态等动态数据
  */
void OLED_Menu_SensorData(void)
{
	// 显示温度数值（2位数字）
	OLED_ShowNum(2, 6, sensorData.temp, 2);
	OLED_ShowChar(2, 8, 'C');    // 显示摄氏度单位

	// 显示湿度数值（2位数字）
	OLED_ShowNum(2, 14, sensorData.humi, 2);
	OLED_ShowChar(2, 16, '%');   // 显示百分号单位

	// 显示光强数值（3位数字，手动模式也实时显示）
	OLED_ShowNum(3, 6, sensorData.lux, 3);

	// 显示亮度百分比数值（3位数字）
	OLED_ShowNum(4, 6, ledDutyRatio, 3);
	OLED_ShowChar(4, 9, '%');    // 显示百分号单位
	OLED_ShowString(1, 15, "  "); // 显示空格字符串
	
	// 显示模式（仅在变化时重绘，避免中文闪烁）
	if (last_systemModel != systemModel)
	{
		last_systemModel = systemModel;  // 更新缓存值
		if (systemModel)
		{
			// 自动模式：显示"自动"（中文编码9-10）
			OLED_ShowChinese(4, 7, 9);
			OLED_ShowChinese(4, 8, 10);
		}
		else
		{
			// 手动模式：显示"手动"（中文编码11-12）
			OLED_ShowChinese(4, 7, 11);
			OLED_ShowChinese(4, 8, 12);
		}
	}
	
	// 显示加湿器状态/档位（仅在变化时重绘）
	if (last_jsq_state != jsq_state)
	{
		last_jsq_state = jsq_state;  // 更新缓存值
	}
	// 根据"香薰"显示内容：开/关以及档位
	// interval=0：显示"开"；interval=1/2/3：显示"一/二/三"；关：显示"关"
	if (jsq_state == 0)
	{
		// 加湿器关闭：显示"关"（中文编码36）
		OLED_ShowChinese(3, 8, 36);
	}
	else
	{
		// 加湿器开启：根据档位显示相应文字
		switch (jsq_interval_mode)
		{
			case 1: OLED_ShowChinese(3, 8, 41); break;  // 一档：显示"一"
			case 2: OLED_ShowChinese(3, 8, 42); break;  // 二档：显示"二"
			case 3: OLED_ShowChinese(3, 8, 43); break;  // 三档：显示"三"
			default: OLED_ShowChinese(3, 8, 35); break; // 常开：显示"开"
		}
	}
}

/**
  * @brief  OLED设置界面显示
  * @param  无
  * @retval 无
  * @note   显示系统设置界面，包括光照阈值和水位状态
  */
void OLED_SetInterfacevoid(void)
{
	// 显示"系统设置界面"标题（中文编码20-25）
	OLED_ShowChinese(1, 2, 20);  // 显示"系"
	OLED_ShowChinese(1, 3, 21);  // 显示"统"
	OLED_ShowChinese(1, 4, 22);  // 显示"设"
	OLED_ShowChinese(1, 5, 23);  // 显示"置"
	OLED_ShowChinese(1, 6, 24);  // 显示"界"
	OLED_ShowChinese(1, 7, 25);  // 显示"面"
	
	// 显示空行分隔
	OLED_ShowString(2, 2, "          ");
	OLED_ShowChar(2, 11, ' ');
	OLED_ShowString(2, 13, "   ");
	
	// 显示"光照阈值"标签（中文编码4-5, 26-27）
	OLED_ShowChinese(3, 2, 4);   // 显示"光"
	OLED_ShowChinese(3, 3, 5);   // 显示"照"
	OLED_ShowChinese(3, 4, 26);  // 显示"阈"
	OLED_ShowChinese(3, 5, 27);  // 显示"值"
	OLED_ShowChar(3, 11, ':');   // 显示冒号

	// 显示"香薰水位"标签（中文编码32-33, 37-38）
	OLED_ShowChinese(4, 2, 32);  // 显示"香"
	OLED_ShowChinese(4, 3, 33);  // 显示"薰"
	OLED_ShowChinese(4, 4, 37);  // 显示"水"
	OLED_ShowChinese(4, 5, 38);  // 显示"位"
	OLED_ShowChar(4, 11, ':');   // 显示冒号
	
	// 显示水位状态：有水/缺水
	if (water_level_stable)
	{
		// 有水状态：显示"有水"（中文编码39-37）
		OLED_ShowChinese(4, 7, 39);
		OLED_ShowChinese(4, 8, 37);
	}
	else
	{
		// 缺水状态：显示"缺水"（中文编码40-37）
		OLED_ShowChinese(4, 7, 40);
		OLED_ShowChinese(4, 8, 37);
	}
	
	// 显示阈值数值（3位数字）
	OLED_ShowNum(3, 13, Sensorthreshold.Illumination_threshold, 3);
	OLED_ShowString(4, 12, " ");  // 显示空格
}

/**
  * @brief  按键选择逻辑
  * @param  无
  * @retval 返回当前选择项编号（2或3）
  * @note   KEY1按键用于在设置项之间切换
  */
uint8_t SetSelection(void)
{
	static uint8_t count = 2;  // 静态变量，保存当前选择项
	
	// 检测KEY1按键
	if(KeyNum == KEY_1)
	{
		KeyNum = 0;     // 清除按键标志
		count++;        // 切换到下一项
		if (count >= 3)
		{
			count = 2;  // 循环回到第一项
		}
	}
	return count;  // 返回当前选择项编号
}

/**
  * @brief  OLED选择符号显示
  * @param  num 选择项编号
  * @retval 无
  * @note   在指定行显示">"选择符号
  */
void OLED_Option(uint8_t num)
{
	switch(num)
	{
		case 1:	// 选择第1项
			OLED_ShowChar(1,1,' ');   // 第1行不显示
			OLED_ShowChar(2,1,' ');   // 第2行不显示
			OLED_ShowChar(3,1,'>');   // 第3行显示选择符号
			OLED_ShowChar(4,1,' ');   // 第4行不显示
			break;
		case 2:	// 选择第2项
			OLED_ShowChar(1,1,' ');
			OLED_ShowChar(2,1,' ');
			OLED_ShowChar(3,1,'>');
			OLED_ShowChar(4,1,' ');
			break;
		case 3:	// 选择第3项
			OLED_ShowChar(1,1,' ');
			OLED_ShowChar(2,1,' ');
			OLED_ShowChar(3,1,'>');
			OLED_ShowChar(4,1,' ');
			break;
		default: break;
	}
}

/**
  * @brief  阈值调节函数
  * @param  num 当前选择项编号
  * @retval 无
  * @note   KEY3增加阈值，KEY4减少阈值
  */
void ThresholdModification(uint8_t num)
{
	switch (num)
	{
		case 1:	break;  // 第1项：暂无功能

		case 2:	// 第2项：光照阈值调节
			// KEY3：增加光照阈值
			if (KeyNum == KEY_3)
			{
				KeyNum = 0;  // 清除按键标志
				Sensorthreshold.Illumination_threshold += 10;  // 阈值增加10
				if (Sensorthreshold.Illumination_threshold > 999)
				{
					Sensorthreshold.Illumination_threshold = 1;  // 超过最大值则回到最小值
				}
				oled_refresh_flag = 1;  // 阈值变化，强制刷新OLED
			}
			// KEY4：减少光照阈值
			else if (KeyNum == KEY_4)
			{
				KeyNum = 0;  // 清除按键标志
				Sensorthreshold.Illumination_threshold -= 10;  // 阈值减少10
				if (Sensorthreshold.Illumination_threshold < 1)
				{
					Sensorthreshold.Illumination_threshold = 999;  // 低于最小值则回到最大值
				}
				oled_refresh_flag = 1;  // 阈值变化，强制刷新OLED
			}
			break;
		case 3:	break;  // 第3项：暂无功能
		default: break;
	}
}

/**
  * @brief  自动模式亮度调节函数
  * @param  无
  * @retval 无
  * @note   根据光照强度自动调节LED亮度：
  *         - 光强低于阈值：亮度100%
  *         - 光强高于阈值：亮度随光强增加而线性降低
  */
void AutoModeAdjust(void)
{
	// 光强低于阈值：亮度设为100%（环境暗，需要补光）
	if (sensorData.lux < Sensorthreshold.Illumination_threshold)
	{
		ledDutyRatio = 100;
	}
	else
	{
		// 光强达到或超过最大值：亮度设为0%（环境足够亮）
		if(sensorData.lux >= 999)
		{
			ledDutyRatio = 0;
		}
		else
		{
			// 计算亮度调节范围
			uint16_t maxRange = 999 - Sensorthreshold.Illumination_threshold;
			if(maxRange == 0)
			{
				ledDutyRatio = 0;  // 防止除零
			}
			else
			{
				// 线性计算亮度：光强越高，亮度越低
				uint16_t delta = sensorData.lux - Sensorthreshold.Illumination_threshold;
				uint32_t temp = (uint32_t)delta * 100 / maxRange;
				ledDutyRatio = 100 - (uint8_t)temp;
			}
		}
	}
	// 设置PWM占空比，更新LED亮度
	TIM_SetCompare1(TIM1, ledDutyRatio);
}

/**
  * @brief  语音模块处理函数
  * @param  无
  * @retval 无
  * @note   解析USART3接收的语音模块指令，执行相应控制操作
  *         串口协议格式：55 AA 02 func cmd xor（XOR=byte0..4异或）
  */
void Asrpro(void)
{
	// 检查是否收到语音模块数据
	if (Usart3_RxFlag == 1)
	{
		Usart3_RxFlag = 0;  // 清除接收标志
		
		// 打印语音模块串口原始数据（6字节）：55 AA len func cmd xor
		// 注意：用于测试映射表时抓包，校验失败也会打印
		printf("[ASR_RAW] %02X %02X %02X %02X %02X %02X\r\n",
			(unsigned)Usart3_RxPacket[0],
			(unsigned)Usart3_RxPacket[1],
			(unsigned)Usart3_RxPacket[2],
			(unsigned)Usart3_RxPacket[3],
			(unsigned)Usart3_RxPacket[4],
			(unsigned)Usart3_RxPacket[5]);
		
		// 语音模块串口协议：55 AA 02 func cmd xor（XOR=byte0..4异或）
		uint8_t xor_calc = CalXorSum(Usart3_RxPacket, 5);
		
		// 先做基础合法性判断：帧头+长度（原始数据会打印出来）
		if ((Usart3_RxPacket[0] != 0x55) || (Usart3_RxPacket[1] != 0xAA) || (Usart3_RxPacket[2] != 0x02))
		{
			return;  // 帧格式错误，直接返回
		}
		
		// XOR偶发错误时仍解析func/cmd，避免指令被丢弃；同时保留错误打印用于定位语音模块端问题
		if (Usart3_RxPacket[5] != xor_calc)
		{
			printf("[ASR_RAW_ERR] xor=%02X calc=%02X\r\n",
				(unsigned)Usart3_RxPacket[5],
				(unsigned)xor_calc);
		}
		
		// 提取功能码和命令码
		uint8_t func = Usart3_RxPacket[3];  // 功能码
		uint8_t cmd  = Usart3_RxPacket[4];  // 命令码
		uint8_t water_ok = (water_level_stable == 1) ? 1 : 0;  // 水位状态

		// ===== 映射表（自拟，可在语音模块侧按此发送） =====
		// func=0x10：模式控制
		//   cmd=0x01 自动模式
		//   cmd=0x00 手动模式
		// func=0x11：灯光档位（仅手动模式生效）
		//   cmd=0x01 一档(15%)
		//   cmd=0x02 二档(30%)
		//   cmd=0x03 三档(60%)
		//   cmd=0x00 关闭(0%)
		// func=0x12：香薰开关
		//   cmd=0x01 打开香薰
		//   cmd=0x00 关闭香薰
		// func=0x13：香薰换挡（间隔模式切换）
		//   cmd=0x01 换挡（0/1/2/3循环）
		// func=0x14：香薰档位（1/2/3档，进入即打开香薰）
		//   cmd=0x01 一档（每10s开2s）
		//   cmd=0x02 二档（每5s开2s）
		//   cmd=0x03 三档（每1s开2s）
		// func=0x15：香薰换挡（1/2/3循环）
		//   cmd=0x01 换挡（1->2->3->1）
		// func=0x16：关闭灯光
		//   cmd=0x00 关闭
		
		// func=0x10：模式控制
		if (func == 0x10)
		{
			if (cmd == 0x01) systemModel = 1;  // 切换到自动模式
			else if (cmd == 0x00) systemModel = 0;  // 切换到手动模式
			ledDutyRatio = 0;  // 切换模式时亮度清零
			key2_press_count = 0;  // 重置KEY2计数器
			TIM_SetCompare1(TIM1, ledDutyRatio);  // 更新PWM输出
			oled_refresh_flag = 1;  // 强制刷新OLED
			printf("[ASR] mode -> %s\r\n", systemModel ? "AUTO" : "MAN");
		}
		// func=0x11：灯光档位控制（仅手动模式生效）
		else if (func == 0x11)
		{
			if (!systemModel)  // 仅手动模式下响应
			{
				switch (cmd)
				{
					case 0x00: ledDutyRatio = 0; break;   // 关闭
					case 0x01: ledDutyRatio = 15; break;  // 一档15%
					case 0x02: ledDutyRatio = 30; break;  // 二档30%
					case 0x03: ledDutyRatio = 60; break;  // 三档60%
					default: break;
				}
				TIM_SetCompare1(TIM1, ledDutyRatio);  // 更新PWM输出
				oled_refresh_flag = 1;  // 强制刷新OLED
				printf("[ASR] light -> %u%%\r\n", (unsigned)ledDutyRatio);
			}
		}
		// func=0x12：香薰开关控制
		else if (func == 0x12)
		{
			if (cmd == 0x01)  // 打开香薰
			{
				if (water_ok)  // 有水才能打开
				{
					jsq_state = 1;  // 设置状态为开启
					jsq_interval_mode = 0;  // 常开模式
					jsq_output_state = 1;  // 输出开启
					jsq_interval_tick = TIM2_GetMs();  // 记录时间戳
					printf("[ASR] jsq -> ON\r\n");
					oled_refresh_flag = 1;
				}
				else
				{
					printf("[ASR] jsq on blocked (water low)\r\n");
				}
			}
			else if (cmd == 0x00)  // 关闭香薰
			{
				jsq_state = 0;  // 设置状态为关闭
				jsq_interval_mode = 0;  // 重置档位
				jsq_output_state = 0;  // 输出关闭
				printf("[ASR] jsq -> OFF\r\n");
				oled_refresh_flag = 1;
			}
		}
		// func=0x13：香薰换挡（间隔模式循环切换）
		else if (func == 0x13)
		{
			if (cmd == 0x01)
			{
				if (water_ok)  // 有水才能换挡
				{
					jsq_interval_mode++;  // 档位+1
					if (jsq_interval_mode >= 4) jsq_interval_mode = 0;  // 循环：3->0
					jsq_interval_tick = TIM2_GetMs();  // 记录时间戳
					// 换挡视为"开启香薰并立即输出一个周期"；换到0视为关闭
					if (jsq_interval_mode == 0)
					{
						jsq_state = 0;
						jsq_output_state = 0;
						JSQ_OFF;  // 关闭加湿器
					}
					else
					{
						jsq_state = 1;
						jsq_output_state = 1;
						JSQ_ON;  // 开启加湿器
					}
					printf("[ASR] jsq interval mode -> %u\r\n", (unsigned)jsq_interval_mode);
					oled_refresh_flag = 1;
				}
				else
				{
					printf("[ASR] jsq interval mode blocked (water low)\r\n");
				}
			}
		}
		// func=0x14：香薰档位直接设置
		else if (func == 0x14)
		{
			if ((cmd >= 0x01) && (cmd <= 0x03))  // 有效档位1-3
			{
				if (water_ok)  // 有水才能设置
				{
					jsq_interval_mode = cmd;  // 设置档位
					jsq_state = 1;  // 开启香薰
					jsq_output_state = 1;  // 输出开启
					jsq_interval_tick = TIM2_GetMs();  // 记录时间戳
					printf("[ASR] jsq grade -> %u\r\n", (unsigned)jsq_interval_mode);
					oled_refresh_flag = 1;
				}
				else
				{
					printf("[ASR] jsq grade blocked (water low)\r\n");
				}
			}
		}
		// func=0x15：香薰换挡（1/2/3循环）
		else if (func == 0x15)
		{
			if (cmd == 0x01)
			{
				if (water_ok)  // 有水才能换挡
				{
					// 如果当前不在1-3档，则从1档开始
					if (jsq_interval_mode < 1 || jsq_interval_mode > 3) jsq_interval_mode = 1;
					else jsq_interval_mode++;  // 否则切换到下一档
					if (jsq_interval_mode > 3) jsq_interval_mode = 1;  // 循环：3->1
					jsq_state = 1;  // 开启香薰
					jsq_output_state = 1;  // 输出开启
					jsq_interval_tick = TIM2_GetMs();  // 记录时间戳
					printf("[ASR] jsq grade cycle -> %u\r\n", (unsigned)jsq_interval_mode);
					oled_refresh_flag = 1;
				}
				else
				{
					printf("[ASR] jsq grade cycle blocked (water low)\r\n");
				}
			}
		}
		// func=0x16：关闭灯光
		else if (func == 0x16)
		{
			if (cmd == 0x00)
			{
				ledDutyRatio = 0;  // 亮度设为0
				key2_press_count = 0;  // 重置KEY2计数器
				TIM_SetCompare1(TIM1, ledDutyRatio);  // 更新PWM输出
				printf("[ASR] light -> OFF\r\n");
				oled_refresh_flag = 1;
			}
		}
	}
}

/**
  * @brief  传感器数据采集函数
  * @param  无
  * @retval 无
  * @note   采集温湿度、光照、水位等传感器数据
  */
void sensorScan(void)
{
	// 读取DHT11温湿度数据
	DHT11_Read_Data(&sensorData.humi, &sensorData.temp);
	// 读取光照强度数据
	LDR_LuxData(&sensorData.lux);
	// 读取水位状态
	sensorData.waterLevel = WaterLevel_IsWaterDetected();
	// 距离传感器（暂未使用）
	sensorData.distance = 0;
	// 人体检测传感器（暂未使用）
	sensorData.people = 0;
}

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   系统初始化后进入主循环，处理传感器采集、按键、显示、控制等
  */
int main(void)
{
	/* ==================== 外设初始化 ==================== */
	ADCX_Init();  // ADC初始化（用于光敏电阻）
	PWM_Init(100 - 1, 720 - 1);  // PWM初始化，频率=72MHz/720/100=1kHz
	Timer2_Init(9,14398);  // TIM2定时器初始化，用于毫秒计时
	Uart1_Init(115200);  // 串口1初始化，用于调试打印
	Uart2_Init(9600);  // 串口2初始化（预留）
	IWDG_Init();  // 独立看门狗初始化
	Uart3_Init();  // 串口3初始化，用于语音模块通信
	LDR_Init();  // 光敏电阻初始化
	OLED_Init();  // OLED显示屏初始化
	DHT11_Init();  // DHT11温湿度传感器初始化
	LED_Init();  // LED灯初始化
	Key_Init();  // 按键初始化
	Buzzer_Init();  // 蜂鸣器初始化
	JSQ_Init();  // 加湿器初始化
	WaterLevel_Init();  // 水位传感器初始化
	
	/* ==================== 加湿器初始状态设置 ==================== */
	// 初始化时强制加湿器为关闭状态
	JSQ_OFF;
	jsq_state = 0;  // 确保状态变量和硬件一致
	
	/* ==================== 参数初始化 ==================== */
	// 从Flash读取保存的光照阈值
	Sensorthreshold.Illumination_threshold = FLASH_R(FLASH_START_ADDR);
	// 距离阈值（暂未使用）
	Sensorthreshold.Distance_threshold = 0;
	
	/* ==================== LED初始亮度设置 ==================== */
	// 手动模式默认亮度设为0
	ledDutyRatio = 0;
	TIM_SetCompare1(TIM1, ledDutyRatio);  // 更新PWM输出
	
	/* ==================== OLED初始显示 ==================== */
	// 初始清屏+绘屏（只执行一次）
	OLED_Clear();  // 清屏
	OLED_Menu();  // 显示主页面静态内容
	OLED_Menu_SensorData();  // 显示传感器数据
	
	/* ==================== 主循环 ==================== */
	while (1)
	{
		/* ========== 软件时基（毫秒） ========== */
		// 注意：使用TIM2维护的ms计数，避免SysTick/Delay等影响节拍稳定性
		uint32_t now_ms = TIM2_GetMs();

		/* ========== 水位状态变量 ========== */
		uint8_t water_ok;   // 有水标志：1=有水，0=缺水
		uint8_t water_low;  // 缺水标志：1=缺水，0=有水

		/* ========== 喂狗 ========== */
		IWDG_ReloadCounter();  // 喂独立看门狗，防止系统复位

		/* ========== 传感器采集 ========== */
		sensorScan();  // 实时采集传感器数据（包括光照）

		/* ========== 语音指令处理（USART3） ========== */
		Asrpro();  // 处理语音模块指令

		/* ========== 更新水位状态（本工程定义：1=有水，0=无水） ========== */
		// 软去抖：连续多次一致才认为状态变化（避免抖动导致误报警/蜂鸣器一直响）
		{
			uint8_t raw = (sensorData.waterLevel == 1) ? 1 : 0;  // 读取原始水位值
			if (raw != water_level_pending)
			{
				// 原始值变化，重置去抖计数
				water_level_pending = raw;
				water_level_cnt = 0;
			}
			else
			{
				// 原始值稳定，累计去抖计数
				if (water_level_cnt < 10) water_level_cnt++;
				if (water_level_cnt >= 5)
				{
					// 连续5次一致，确认状态变化
					water_level_stable = water_level_pending;
				}
			}
		}
		water_ok = (water_level_stable == 1) ? 1 : 0;   // 有水标志
		water_low = (water_ok == 0) ? 1 : 0;           // 缺水标志

		/* ========== 串口1调试打印：周期性状态输出 ========== */
		if ((now_ms - debug_tick) >= 1000)  // 每1秒打印一次
		{
			debug_tick = now_ms;
			printf("[STAT] t=%lu ms, mode=%s, T=%uC, H=%u%%, lux=%u, led=%u%%, water=%s, jsq=%s, interval=%u\r\n",
				now_ms,
				systemModel ? "AUTO" : "MAN",
				sensorData.temp,
				sensorData.humi,
				sensorData.lux,
				ledDutyRatio,
				water_ok ? "OK" : "LOW",
				jsq_state ? "ON" : "OFF",
				(unsigned)jsq_interval_mode);
		}

		/* ========== 串口1调试打印：缺水从无->有（边沿）事件 ========== */
		if ((water_low != 0) && (last_water_low == 0))
		{
			// 检测到缺水边沿事件，发送报警指令给语音模块
			Usart3_SendPacket(0x20, 0x01);
			printf("[ALARM] water low -> force jsq off\r\n");
		}
		last_water_low = water_low;  // 更新上一次缺水状态

		/* ========== 缺水保护：强制关加湿器 + 蜂鸣器周期提示 ========== */
		if (water_low)
		{
			// 缺水时强制关闭加湿器
			if (jsq_state != 0)
			{
				jsq_state = 0;
				oled_refresh_flag = 1;  // 强制刷新OLED
			}
			jsq_output_state = 0;
			JSQ_OFF;  // 关闭加湿器硬件输出

			// 蜂鸣器周期性提示（每2秒响100ms）
			if ((now_ms - buzzer_tick) >= 2000)
			{
				buzzer_tick = now_ms;
				buzzer_on_tick = now_ms;
				Buzzer_ON();  // 开启蜂鸣器
			}
			// 蜂鸣器响100ms后关闭
			if ((buzzer_on_tick != 0) && ((now_ms - buzzer_on_tick) >= 100))
			{
				buzzer_on_tick = 0;
				Buzzer_OFF();  // 关闭蜂鸣器
			}
		}
		else
		{
			// 有水时重置蜂鸣器计时
			buzzer_tick = now_ms;
			buzzer_on_tick = 0;
			Buzzer_OFF();  // 确保蜂鸣器关闭
		}

		/* ========== 加湿器输出控制（间隔模式） ========== */
		// 只有在water_ok时才允许控制输出；缺水时上面已强制关闭
		if (water_ok)
		{
			// interval_mode=0：常开（由jsq_state控制开/关）
			// interval_mode=1：喷2s停10s循环
			// interval_mode=2：喷2s停5s循环
			// interval_mode=3：喷2s停1s循环
			static const uint32_t off_table_ms[4] = {0, 10000, 5000, 1000};  // 各档位停止时间
			uint32_t off_ms = off_table_ms[jsq_interval_mode & 0x03];  // 获取当前档位的停止时间
			uint32_t on_ms = 2000;  // 喷雾持续时间2秒

			if (jsq_state == 0)
			{
				// 加湿器关闭状态
				jsq_output_state = 0;
				JSQ_OFF;
				jsq_interval_tick = now_ms;
			}
			else
			{
				// 加湿器开启状态
				if (off_ms == 0)
				{
					// 常开模式
					jsq_output_state = 1;
					JSQ_ON;
				}
				else
				{
					// 间隔模式：根据时间切换开/关状态
					if (jsq_output_state == 0)
					{
						// 当前处于关闭状态，检查是否到达开启时间
						if ((now_ms - jsq_interval_tick) >= off_ms)
						{
							jsq_output_state = 1;
							jsq_interval_tick = now_ms;
							JSQ_ON;  // 开启加湿器
						}
						else
						{
							JSQ_OFF;  // 保持关闭
						}
					}
					else
					{
						// 当前处于开启状态，检查是否到达关闭时间
						if ((now_ms - jsq_interval_tick) >= on_ms)
						{
							jsq_output_state = 0;
							jsq_interval_tick = now_ms;
							JSQ_OFF;  // 关闭加湿器
						}
						else
						{
							JSQ_ON;  // 保持开启
						}
					}
				}
			}
		}

		/* ========== OLED刷新节拍（200ms一次，手动/自动都刷新） ========== */
		// 页面切换时只清屏一次，并在新页面首次刷新时绘制静态UI
		if (menu != last_menu)
		{
			OLED_Clear();  // 切换页面时清屏
			display_static_drawn = 0;  // 重置静态内容绘制标志
			last_menu = menu;
			// 清屏后必须重绘动态中文（否则last_*缓存会导致"手动/自动""开/关"不显示）
			last_systemModel = 0xFF;
			last_jsq_state = 0xFF;
			oled_refresh_flag = 1;
		}

		// 每200ms刷新一次OLED，或强制刷新标志为1时刷新
		if(((now_ms - oled_tick) >= 200) || oled_refresh_flag == 1)
		{
			oled_tick = now_ms;
			oled_refresh_flag = 0;  // 清空强制刷新标志
			
			switch (menu)
			{
				case display_page:
					// 主显示页：静态布局只绘制一次，后续仅更新数据区，避免中文重绘闪烁
					if (display_static_drawn == 0)
					{
						OLED_Menu();  // 绘制静态UI
						display_static_drawn = 1;
					}
					OLED_Menu_SensorData();  // 每次刷新都重新绘制传感器数据
					break;
					
				case settingsPage:
					// 设置页：此函数内部包含阈值数值显示，因此仍按刷新节拍更新
					OLED_SetInterfacevoid();  // 绘制设置界面
					OLED_Option(SetSelection());  // 绘制选择符号
					break;
					
				case timeSettingsPage:
					// 时间设置页（预留）：直接返回设置页
					OLED_Clear();
					menu = settingsPage;
					break;
			}
		}

		/* ========== 功能逻辑（按菜单页处理） ========== */
		switch (menu)
		{
			case display_page:
				/* ===== 自动模式 ===== */
				if (systemModel)
				{
					AutoModeAdjust();  // 自动调节亮度
					oled_refresh_flag = 1;  // 自动模式亮度变化，强制刷新
				}
				/* ===== 手动模式 ===== */
				else
				{
					// 屏蔽原有手动按键函数，不再使用
					// LED_PWM_KEY();
					// LED_APP_Command();
					
					// 手动模式：仅处理KEY2的循环亮度逻辑
					if(KeyNum == KEY_2)
					{
						KeyNum = 0;  // 清除按键标志，避免重复触发
						key2_press_count++;  // 按下次数+1
						
						// 根据次数设置亮度（循环：15%->30%->60%->0%）
						switch(key2_press_count)
						{
							case 1: ledDutyRatio = 15;  break;  // 第1次：15%
							case 2: ledDutyRatio = 30;  break;  // 第2次：30%
							case 3: ledDutyRatio = 60;  break;  // 第3次：60%
							case 4:
								ledDutyRatio = 0;        // 第4次：0%
								key2_press_count = 0;    // 重置计数器，实现循环
								break;
							default:
								ledDutyRatio = 0;
								key2_press_count = 0;
								break;
						}
						
						TIM_SetCompare1(TIM1, ledDutyRatio);  // 更新PWM占空比，改变亮度
						printf("[KEY] KEY2 brightness -> %u%%\r\n", (unsigned)ledDutyRatio);
						oled_refresh_flag = 1;  // 强制刷新OLED，显示最新亮度
					}
					
					// 不再每次循环都强制刷新OLED，避免界面闪烁；
					// 仅在按键/状态变化时置oled_refresh_flag。
				}
				
				/* ===== KEY1：切换模式 ===== */
				if (KeyNum == KEY_1)
				{
					KeyNum = 0;
					systemModel = ~systemModel;  // 切换模式（取反）
					ledDutyRatio = 0;  // 切换模式时亮度强制为0
					key2_press_count = 0;  // 重置KEY2计数器，避免次数残留
					TIM_SetCompare1(TIM1, ledDutyRatio);  // 更新PWM输出
					OLED_Clear();  // 仅切换模式时清屏一次
					display_static_drawn = 0;  // 重置静态内容绘制标志
					printf("[KEY] KEY1 mode -> %s\r\n", systemModel ? "AUTO" : "MAN");
					oled_refresh_flag = 1;
				}

				/* ===== KEY3：加湿器开关（缺水时禁止打开） ===== */
				if (KeyNum == KEY_3)
				{
					KeyNum = 0;
					if (water_ok)  // 有水才能操作
					{
						jsq_state = !jsq_state;  // 切换加湿器状态
						printf("[KEY] KEY3 jsq -> %s\r\n", jsq_state ? "ON" : "OFF");
						oled_refresh_flag = 1;
					}
					else
					{
						printf("[KEY] KEY3 jsq blocked (water low)\r\n");
					}
				}

				/* ===== KEY4：加湿器间隔档位切换 ===== */
				if (KeyNum == KEY_4)
				{
					KeyNum = 0;
					if (water_ok)  // 有水才能操作
					{
						jsq_interval_mode++;  // 档位+1
						if (jsq_interval_mode >= 4)
						{
							jsq_interval_mode = 0;  // 循环：3->0
						}
						jsq_interval_tick = now_ms;  // 记录时间戳
						// KEY4换挡视为"开启香薰并立即输出一个周期"；换到0视为关闭
						if (jsq_interval_mode == 0)
						{
							jsq_state = 0;
							jsq_output_state = 0;
							JSQ_OFF;
						}
						else
						{
							jsq_state = 1;
							jsq_output_state = 1;
							JSQ_ON;
						}
						printf("[KEY] KEY4 interval mode -> %u\r\n", (unsigned)jsq_interval_mode);
						oled_refresh_flag = 1;
					}
					else
					{
						printf("[KEY] KEY4 interval mode blocked (water low)\r\n");
					}
				}
				
				/* ===== 长按KEY1：进入设置界面 ===== */
				if (KeyNum == KEY_Long1)
				{
					KeyNum = 0;
					OLED_Clear();  // 仅切换界面时清屏一次
					display_static_drawn = 0;
					menu = settingsPage;  // 切换到设置页
					oled_refresh_flag = 1;
				}
				break;
				
			case settingsPage:
				/* ===== 设置页：调节阈值 ===== */
				ThresholdModification(SetSelection());
			
				/* ===== KEY2：保存退出 ===== */
				if (KeyNum == KEY_2)
				{
					KeyNum = 0;
					// 保存阈值到Flash
					FLASH_W(FLASH_START_ADDR, Sensorthreshold.Illumination_threshold, 0);
					OLED_Clear();  // 仅切换界面时清屏一次
					display_static_drawn = 0;
					menu = display_page;  // 返回显示页
					oled_refresh_flag = 1;
				}
				break;
				
			case timeSettingsPage:
				/* ===== 时间设置页（预留） ===== */
				OLED_Clear();
				display_static_drawn = 0;
				menu = settingsPage;
				oled_refresh_flag = 1;
				break;
		}
	}
}
