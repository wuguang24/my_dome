#ifndef __ACTUATOR_H
#define __ACTUATOR_H

#include "stm32f10x.h"

// 执行器模块接口（风扇/补光灯/加湿器）
// 注意：具体引脚与有效电平在 Actuator.c 内部封装，上层只使用 on/off 语义。
void Actuator_Init(void);

// on=1 打开，on=0 关闭
void Actuator_SetFan(uint8_t on);
void Actuator_SetLight(uint8_t on);
void Actuator_SetHumidifier(uint8_t on);

// 获取当前输出状态（返回 0/1）
uint8_t Actuator_GetFan(void);
uint8_t Actuator_GetLight(void);
uint8_t Actuator_GetHumidifier(void);

#endif
