#ifndef __DELAY_H                 // 防止头文件重复包含
#define __DELAY_H                 // 头文件宏定义开始

void Delay_us(uint32_t us);       // 微秒级延时函数声明（基于SysTick）
void Delay_ms(uint32_t ms);       // 毫秒级延时函数声明（内部调用Delay_us）
void Delay_s(uint32_t s);         // 秒级延时函数声明（内部调用Delay_ms）

#endif                            // 头文件宏定义结束
