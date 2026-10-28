#ifndef __GY302_H
#define __GY302_H

#include "stm32f10x.h"

// GY-302 光照模块（BH1750）接口定义
// BH1750 地址（由模块 ADDR 引脚决定）：
// - 悬空/接 VCC：0x23（常见）
// - 接 GND：0x5C
#define BH1750_ADDR          0x23      // ADDR引脚悬空或接VCC
// #define BH1750_ADDR        0x5C      // ADDR引脚接地

// BH1750 命令集（部分）
// 说明：不同模式的转换时间不同，连续/一次模式的区别在于是否自动持续测量。
#define BH1750_POWER_DOWN    0x00      // 关闭模块
#define BH1750_POWER_ON      0x01      // 打开模块
#define BH1750_RESET         0x07      // 重置数据寄存器
#define BH1750_CONT_H_MODE   0x10      // 连续高分辨率模式，120ms
#define BH1750_CONT_H2_MODE  0x11      // 连续高分辨率模式2，120ms
#define BH1750_CONT_L_MODE   0x13      // 连续低分辨率模式，16ms
#define BH1750_ONE_H_MODE    0x20      // 一次高分辨率模式，120ms
#define BH1750_ONE_H2_MODE   0x21      // 一次高分辨率模式2，120ms
#define BH1750_ONE_L_MODE    0x23      // 一次低分辨率模式，16ms

// 函数声明
// 初始化：上电/复位/设置测量模式
void GY302_Init(void);
// 读取光照强度（单位：lx）；失败返回 0.0
float GY302_ReadLight(void);

#endif
