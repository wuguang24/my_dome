/*
 * 模块名称: 按键输入驱动 (KEY)
 * 功能概述:
 *   - 初始化 KEY0/WK_UP 引脚为下拉输入
 *   - 提供按键扫描函数，支持去抖与连按模式
 * 硬件映射:
 *   - KEY0: PC0 下拉输入
 *   - WK_UP: PA0 下拉输入
 */
#ifndef _KEY_H
#define _KEY_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F407ZGT6开发板
//按键输入驱动代码	   
//版本：V1.0											  
//////////////////////////////////////////////////////////////////////////////////

// 方式一: 位带操作读取IO(本工程未使用，保留参考)
//#define KEY0        PEin(4) //KEY0按键PE4
//#define WK_UP       PAin(0) //WKUP按键PA0


// 方式二: HAL 读取IO(当前使用): PC0/PA0 为下拉输入，高电平表示按下
#define KEY0        HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)  // KEY0 按键 PC0
#define WK_UP       HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)  // WKUP 按键 PA0

// 扫描返回值定义
#define KEY0_PRES 	1
#define WKUP_PRES   2

void KEY_Init(void);     // 按键初始化函数: 使能时钟/配置GPIO为下拉输入
u8 KEY_Scan(u8 mode);    // 按键扫描函数: mode=0非连按 / mode=1连按
#endif
