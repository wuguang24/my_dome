#ifndef __WIND_DETECT_H
#define __WIND_DETECT_H

#include <stdint.h>

// 阈值
#define WIND_HIGH  8.0f    // ≥8m/s 关窗
#define WIND_LOW   5.0f    // <5m/s 开窗
#define WIND_CD_60 60      // 60秒倒计时
#define WIND_WAIT_30 30    // 30秒等待

// 函数
uint8_t wind_chk(float wind);                    // 检查风速等级
void wind_ctrl(float wind, uint8_t fire, uint8_t rain, 
               uint8_t* wind_closed, uint16_t* wind_cd, 
               uint8_t* wind_wait, uint16_t* wind_wait_cd);

#endif
