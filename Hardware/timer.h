#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"

// 定时器接口（对应 timer.c）
// TIM3 在中断中产生软件节拍标志：
// - flag_100ms：每 100ms 置 1（主循环消费后清零）
// - flag_1s   ：每 1s 置 1（主循环消费后清零）
// 使用建议：主循环里 if(flag_xxx){flag_xxx=0; ...}

// 全局标志
extern volatile uint8_t flag_100ms;  
extern volatile uint8_t flag_1s;     

// 倒计时
// rain_cd：以 1 秒为单位递减（在 TIM3 中断的 1s 节拍里维护）
extern volatile uint16_t rain_cd;    // 雨倒计时（秒）

// 火灾标志
extern volatile uint8_t fire_flag;   // 1=报警，0=正常

// 初始化 TIM3
void TIM3_Init(void);

#endif
