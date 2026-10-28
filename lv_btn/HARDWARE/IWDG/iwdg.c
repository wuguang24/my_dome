/*
 * 文件名称: iwdg.c
 * 模块功能: 独立看门狗(IWDG)驱动
 * 功能说明:
 *   - 初始化独立看门狗
 *   - 提供喂狗接口
 *   - 防止系统死机
 * 工作原理:
 *   - IWDG使用独立的32kHz LSI时钟
 *   - 计数器递减到0时产生复位
 *   - 必须定期喂狗防止复位
 * 超时计算:
 *   Tout = ((4 * 2^prer) * rlr) / 32 (ms)
 *   例如: prer=64, rlr=2500
 *   Tout = (4 * 64 * 2500) / 32 = 20000ms = 20秒
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 * 日期: 2025-12-02
 */

#include "iwdg.h"

/* 独立看门狗句柄 */
IWDG_HandleTypeDef IWDG_Handler;

/**
 * @brief  初始化独立看门狗
 * @param  prer: 分频系数
 *         - IWDG_PRESCALER_4   (分频4)
 *         - IWDG_PRESCALER_8   (分频8)
 *         - IWDG_PRESCALER_16  (分频16)
 *         - IWDG_PRESCALER_32  (分频32)
 *         - IWDG_PRESCALER_64  (分频64)
 *         - IWDG_PRESCALER_128 (分频128)
 *         - IWDG_PRESCALER_256 (分频256)
 * @param  rlr: 自动重装载值，范围0~0xFFF (0~4095)
 * @retval None
 * @note   超时时间计算: Tout = ((4 * 2^prer) * rlr) / 32 (ms)
 *         初始化后看门狗自动启动，必须定期喂狗
 *         示例: prer=64(IWDG_PRESCALER_64), rlr=2500
 *               Tout = (4 * 64 * 2500) / 32 = 20000ms = 20秒
 */
void IWDG_Init(u8 prer,u16 rlr)
{
    IWDG_Handler.Instance = IWDG;            // 独立看门狗外设
    IWDG_Handler.Init.Prescaler = prer;      // 设置分频系数
    IWDG_Handler.Init.Reload = rlr;          // 设置重装载值
    HAL_IWDG_Init(&IWDG_Handler);            // 初始化IWDG(自动启动)
}
    
/**
 * @brief  喂独立看门狗
 * @param  None
 * @retval None
 * @note   必须在超时时间内调用此函数，否则系统复位
 *         建议喂狗周期 < 超时时间的一半
 *         例如: 超时20秒，建议每10秒喂一次狗
 */
void IWDG_Feed(void)
{   
    HAL_IWDG_Refresh(&IWDG_Handler);         // 重新加载计数器(喂狗)
}
