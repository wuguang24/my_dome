#ifndef __FIRE_DETECT_H
#define __FIRE_DETECT_H

#include "stm32f10x.h"

//// 火灾阈值
//#define SMK_FIRE   1200  // 烟雾 > 1200
//#define RIS_FIRE   12     // 温升 > 12°C/分钟
//#define TEMP_FIRE  70    // 温度 > 26°C

//// 正常阈值
//#define SMK_NOR    800   // 烟雾 < 800
//#define RIS_NOR    5     // 温升 < 5°C/分钟
//#define TEMP_NOR   50    // 温度 < 50°C

// 火灾阈值
#define SMK_FIRE   50     // 烟雾 > 50
#define RIS_FIRE   3      // 温升 > 3°C/分钟
#define TEMP_FIRE  23     // 温度 > 30°C

// 正常阈值
#define SMK_NOR    30     // 烟雾 < 30
#define RIS_NOR    2      // 温升 < 2°C/分钟
#define TEMP_NOR   20     // 温度 < 25°C

#define FIRE_DELAY 30     // 火灾延时30秒

// 火灾检测函数
uint8_t fire_chk(float smk, uint8_t t, int8_t rise);
uint8_t nor_chk(float smk, uint8_t t, int8_t rise);
uint8_t flt_chk(float smk, uint8_t t);

#endif
