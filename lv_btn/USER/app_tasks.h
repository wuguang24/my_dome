/**
 ******************************************************************************
 * @file    app_tasks.h
 * @brief   应用任务管理头文件
 *          包含所有FreeRTOS任务的声明和配置
 ******************************************************************************
 */

#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"

/* 全局：SAR 模式标志，供 UI 读取 */
extern volatile uint8_t g_sar_mode;

/* 全局：当前UI页面索引，供按键切换 */
extern volatile uint8_t g_ui_page_index;

/* 任务初始化函数 */
void App_Tasks_Init(void);

/* 任务函数声明 */
void start_task(void *pvParameters);
void lv_demo_task(void *pvParameters);
void radar_task(void *pvParameters);
void gps_task(void *pvParameters);
void dht_task(void *pvParameters);
void control_task(void *pvParameters);
void watchdog_task(void *pvParameters);
void led_task(void *pvParameters);

#endif /* __APP_TASKS_H */
