/**
 * 模块名称: LVGL演示入口 (lvgl_demo.c)
 * 功能概述:
 *   - 提供LVGL演示的入口函数
 *   - 所有RTOS任务调度已移至app_tasks.c
 *   - UI界面代码在lv_demo.c中
 */
/************************************************
 STM32F407ZGT6开发板
 LVGL 演示
************************************************/
#include "lvgl_demo.h"
#include "app_tasks.h"

/**
 * @brief       lvgl_demo入口函数
 * @param       无
 * @retval      无
 * @note        所有RTOS任务调度已移至app_tasks.c
 */
void lvgl_demo(void)
{
    /* 初始化并启动所有应用任务 */
    App_Tasks_Init();
    
    /* 注意：以下代码不会执行，因为vTaskStartScheduler()不会返回 */
}

/* 以下代码已移至app_tasks.c */
