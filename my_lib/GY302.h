#ifndef __GY302_H
#define __GY302_H

#include "stm32f10x.h"

// BH1750（GY302）传感器地址定义（根据ADDR引脚选择）
#define BH1750_ADDR          0x23      // ADDR引脚悬空或接VCC
// #define BH1750_ADDR        0x5C      // ADDR引脚接地

// BH1750命令码
#define BH1750_POWER_DOWN    0x00      // 关闭模式
#define BH1750_POWER_ON      0x01      // 开模式
#define BH1750_RESET         0x07      // 重置数据寄存器
#define BH1750_CONT_H_MODE   0x10      // 连续高分辨率模式（120ms）
#define BH1750_CONT_H2_MODE  0x11      // 连续高分辨率模式2（120ms）
#define BH1750_CONT_L_MODE   0x13      // 连续低分辨率模式（16ms）
#define BH1750_ONE_H_MODE    0x20      // 一次高分辨率模式（120ms）
#define BH1750_ONE_H2_MODE   0x21      // 一次高分辨率模式2（120ms）
#define BH1750_ONE_L_MODE    0x23      // 一次低分辨率模式（16ms）

// 函数声明
// GY302光照传感器初始化函数
void GY302_Init(void);
// 读取光照强度函数
// 返回值：光照强度值（单位：lx）
float GY302_ReadLight(void);

#endif
