#ifndef __JW01_H
#define __JW01_H

#include "stm32f10x.h"

// JW01 CO2 传感器接口（对应 JW01.c）
// 使用方式（典型）：
// 1) 上电后调用 JW01_Init()
// 2) 主循环周期性调用 JW01_ProcessData() 解析串口缓冲
// 3) 通过 JW01_NewDataAvailable() 判断是否有新数据，再用 JW01_GetCO2_PPM() 取值

// 初始化函数
void JW01_Init(void);

// 数据处理函数（需在主循环中调用）
void JW01_ProcessData(void);

// 检查是否有新的 CO2 数据
uint8_t JW01_NewDataAvailable(void);

// 获取 CO2 浓度值（ppm）
// 注意：读取后会清除“新数据”标志。
uint16_t JW01_GetCO2_PPM(void);

// 调试变量声明：便于在调试器中查看原始帧、校验和与有效性
extern uint8_t JW01_Debug_RawData[6];
extern uint8_t JW01_Debug_Checksum;
extern uint8_t JW01_Debug_Valid;

#endif
