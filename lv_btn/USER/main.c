/*
 * 文件名称: main.c
 * 模块功能: 系统主程序入口
 * 功能说明:
 *   - 系统时钟配置(168MHz)
 *   - 外设初始化(LCD/触摸/LED/按键等)
 *   - 传感器初始化(雷达/GPS/温湿度)
 *   - 通信初始化(ESP32串口)
 *   - 启动FreeRTOS任务调度
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 * 日期: 2025-12-02
 */

/* 系统头文件 */
#include "sys.h"
#include "delay.h"
#include "usart.h"


/* 外设驱动头文件 */
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "touch.h"
#include "timer.h"
#include "beep.h"

/* LVGL图形库头文件 */
#include "lvgl.h"
#include "lv_port_indev_template.h"
#include "lv_port_disp_template.h"
#include "lvgl_demo.h"

/* 传感器和通信模块头文件 */
#include "radar.h"
#include "gps.h"
#include "esp32_uart.h"
#include "dht11.h"

/**
 * @brief  主函数 - 系统入口
 * @param  None
 * @retval int (实际不会返回，进入RTOS调度后不再返回)
 * @note   初始化顺序:
 *         1. HAL库和系统时钟
 *         2. 基础外设(延时/串口/LED/按键)
 *         3. 显示系统(LCD/触摸屏)
 *         4. 传感器模块(雷达/GPS/温湿度)
 *         5. 通信模块(ESP32)
 *         6. 启动RTOS任务调度
 */
int main(void)
{     
	/* ========== 步骤1: HAL库和系统时钟初始化 ========== */
	HAL_Init();                   	    // 初始化HAL库(中断优先级分组等)
	STM32_Clock_Init(336,25,2,7);  	    // 配置系统时钟为168MHz
	                                    // PLL参数: M=25, N=336, P=2, Q=7
	                                    // 计算: (25MHz / 25) * 336 / 2 = 168MHz
	
	/* ========== 步骤2: 基础外设初始化 ========== */
	delay_init(168);               	    // 初始化延时函数(基于168MHz系统时钟)
	uart_init(115200);             	    // 初始化串口1(调试用, 115200bps)
	printf("boot ok\r\n");              // 输出启动信息
	LED_Init();						    // 初始化LED(LED0:PA1 PWM, LED1:PA2 GPIO)
	KEY_Init();						    // 初始化按键(KEY0:PE4, KEY_UP:PA0)
	BEEP_Init();                        // 初始化蜂鸣器(PG13, 低电平响)
	
	/* ========== 步骤3: 显示系统初始化 ========== */
	LCD_Init();                         // 初始化LCD显示屏(ILI9341, 240x320)
	tp_dev.init();				        // 初始化触摸屏(电容屏/电阻屏自适应)
	
	/* ========== 步骤4: 定时器初始化 ========== */
	TIM5_Init(999, 83);                 // 初始化TIM5定时器(1ms中断，用于GPS帧检测)
	                                    // 计算: (999+1)*(83+1)/84MHz = 1ms
	
	/* 注意: 传感器和ESP32初始化已移至app_tasks.c的start_task中
	 * 避免重复初始化导致的问题 */
	
	/* ========== 步骤6: 启动RTOS任务调度 ========== */
	lvgl_demo();                        // 创建FreeRTOS任务并启动调度器
	                                    // 注意: 此函数内部调用vTaskStartScheduler()
	                                    // 正常情况下不会返回
	
	/* 如果调度器异常退出，程序会执行到这里(不应该发生) */
	while(1);
}
