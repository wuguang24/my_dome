#ifndef __LED_H                // 防止头文件被重复包含
#define __LED_H                // 头文件宏定义开始


void LED_Init(void);           // 初始化LED相关GPIO（PC13/PC14）
//void LED_G_ON(void);         // 预留：绿色LED点亮（当前未使用）
//void LED_G_OFF(void);        // 预留：绿色LED熄灭（当前未使用）
//void LED_G_Turn(void);       // 预留：绿色LED翻转（当前未使用）
void LED_R_ON(void);           // 红灯点亮（PC13，低电平点亮）
void LED_R_OFF(void);          // 红灯熄灭（PC13，高电平熄灭）
void LED_R_Turn(void);         // 红灯翻转（读取输出电平后翻转）
void LED_B_ON(void);           // 蓝灯点亮（PC14，低电平点亮）
void LED_B_OFF(void);          // 蓝灯熄灭（PC14，高电平熄灭）
void LED_B_Turn(void);         // 蓝灯翻转（读取输出电平后翻转）

#endif                         // 头文件宏定义结束
