#ifndef __RAIN_DETECT_H
#define __RAIN_DETECT_H

#include "stm32f10x.h"

#define RAIN_SML     100    // 小雨阈值
#define RAIN_BIG     300   // 大雨阈值
#define RAIN_WAIT    180     // 雨量等待时间（秒）
#define RAIN_DELAY   180000  // 雨量延时180秒（毫秒）

// 雨量级别
uint8_t rain_lvl(uint16_t adc);

// 雨量控制（修改后的函数）
void rain_ctrl(uint8_t lvl, uint8_t fire, uint32_t time, uint8_t* win_closed, 
               uint32_t* timer, uint16_t* rain_cd, uint8_t* rain_on);

#endif
