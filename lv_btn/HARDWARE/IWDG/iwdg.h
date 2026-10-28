/*
 * 模块名称: 独立看门狗驱动 (iwdg)
 * 功能概述:
 *   - 初始化独立看门狗(IWDG)
 *   - 提供喂狗接口，防止系统死机
 * 
 * 工作原理:
 *   - IWDG使用独立的32kHz LSI时钟
 *   - 计数器从重装载值递减到0时产生系统复位
 *   - 必须在超时前调用喂狗函数重置计数器
 * 
 * 超时计算:
 *   Tout = ((4 * 2^prer) * rlr) / 32 (ms)
 *   示例: prer=IWDG_PRESCALER_64(64分频), rlr=2500
 *         Tout = (4 * 64 * 2500) / 32 = 20000ms = 20秒
 * 
 * 使用方法:
 *   1. 调用 IWDG_Init(IWDG_PRESCALER_64, 2500) 初始化20秒超时
 *   2. 在主循环或定时任务中定期调用 IWDG_Feed() 喂狗
 *   3. 建议喂狗周期 < 超时时间的一半
 * 
 * 注意事项:
 *   - 一旦启动看门狗，无法关闭(只能复位)
 *   - 调试时可能需要禁用看门狗
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef __IWDG_H
#define __IWDG_H
#include "sys.h"

void IWDG_Init(u8 prer, u16 rlr);  // 初始化IWDG并使能
void IWDG_Feed(void);              // 喂狗(重载计数器)

#endif
