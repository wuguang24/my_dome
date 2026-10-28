/*
 * 文件名称: timer.c
 * 模块功能: 通用定时器中断驱动
 * 功能说明:
 *   - 配置TIM5定时器产生周期性中断
 *   - 可用于LVGL心跳或LED闪烁等周期性任务
 * 硬件资源:
 *   - TIM5: 32位通用定时器，挂载在APB1总线(84MHz)
 * 中断优先级:
 *   - 抢占优先级: 1
 *   - 子优先级: 3
 * 定时计算:
 *   Tout = ((arr+1) * (psc+1)) / Ft (us)
 *   其中Ft为定时器时钟频率(MHz)
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 * 日期: 2025-12-02
 */

#include "timer.h"
#include "led.h"
#include "gps.h"
//#include "lvgl.h"

/* 定时器句柄 */
TIM_HandleTypeDef TIM5_Handler;

/**
 * @brief  初始化TIM5定时器中断
 * @param  arr: 自动重装载值，范围0~0xFFFFFFFF (TIM5是32位定时器)
 * @param  psc: 预分频系数，范围0~0xFFFF
 * @retval None
 * @note   定时周期计算: Tout = ((arr+1) * (psc+1)) / Ft (us)
 *         其中Ft为定时器时钟频率(MHz)
 *         TIM5时钟源: APB1 = 84MHz
 *         示例: arr=999, psc=83
 *               Tout = (1000 * 84) / 84 = 1000us = 1ms
 */
void TIM5_Init(u16 arr,u16 psc)
{  
    /* 配置定时器基本参数 */
    TIM5_Handler.Instance = TIM5;                           // 定时器5
    TIM5_Handler.Init.Prescaler = psc;                      // 预分频系数
    TIM5_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;     // 向上计数模式
    TIM5_Handler.Init.Period = arr;                         // 自动重装载值
    TIM5_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // 时钟分频因子
    HAL_TIM_Base_Init(&TIM5_Handler);                       // 初始化定时器
    
    HAL_TIM_Base_Start_IT(&TIM5_Handler);                   // 使能定时器和更新中断
}

/**
 * @brief  定时器底层初始化(MSP)
 * @param  htim: 定时器句柄指针
 * @retval None
 * @note   此函数由HAL_TIM_Base_Init()自动调用
 *         功能: 使能时钟、配置中断优先级
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM5)                              // 判断是否为TIM5
	{
		__HAL_RCC_TIM5_CLK_ENABLE();                        // 使能TIM5时钟
		HAL_NVIC_SetPriority(TIM5_IRQn, 6, 0);              // 设置中断优先级(>=5才能调用FreeRTOS API)
		HAL_NVIC_EnableIRQ(TIM5_IRQn);                      // 使能TIM5中断
	}
}

/**
 * @brief  TIM5中断服务函数
 * @param  None
 * @retval None
 * @note   定时器溢出时自动调用
 *         调用HAL库中断处理函数，然后触发回调函数
 */
void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM5_Handler);                      // HAL库中断处理
}

/**
 * @brief  定时器周期回调函数
 * @param  htim: 定时器句柄指针
 * @retval None
 * @note   定时器溢出时由HAL_TIM_IRQHandler()调用
 *         可在此处添加周期性任务:
 *         - LVGL心跳: lv_tick_inc(1)
 *         - LED闪烁: LED翻转
 *         - 其他周期性任务
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == (&TIM5_Handler))                             // 判断是否为TIM5中断
    {
		// lv_tick_inc(1);                                  // LVGL的1ms心跳(已禁用)
        LED1 = !LED1;                                       // LED1状态翻转(闪烁指示)
        GPS_Frame_Check();                                  // GPS帧超时检测(1ms调用)
    }
}
