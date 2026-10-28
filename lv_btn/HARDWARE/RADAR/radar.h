/*
 * 模块名称: 雷达数据解析与状态缓存 (RADAR)
 * 功能概述:
 *   - 通过 USART3 接收并解析雷达协议帧
 *   - 维护最新雷达状态快照供其他任务读取
 *   - 提供初始化/处理/获取状态接口
 */
#ifndef __RADAR_H
#define __RADAR_H

#include "sys.h"
#include "usart3.h"

// 雷达状态结构体
typedef struct {
    // 存在/运动
    int8_t exist;              // 0 无人, 1 有人, -1 未知
    int8_t movement;           // 0 无, 1 静止, 2 活跃, -1 未知
    int16_t distance_cm;       // 0~65535, -1 未知
    int8_t body_motion;        // 0~100, -1 未知
    int16_t pos_x_cm;          // 有符号, -32768~32767, 32767 表示未知
    int16_t pos_y_cm;
    int16_t pos_z_cm;

    // 呼吸/心率
    int8_t respiration;        // 0~35, -1 未知
    int8_t respiration_state;  // 1 正常,2 过高,3 过低,4 无, -1 未知
    int8_t heart_rate;         // 0~255(期望60~120), -1 未知

    // 睡眠
    int8_t in_bed;             // 0 离床,1 入床,2 无, -1 未知
    int8_t sleep_state;        // 0 深睡,1 浅睡,2 清醒,3 无, -1 未知

    // 十分钟综合
    int8_t sum_exist;          // 1 有人 0 无人 -1 未知
    int8_t sum_sleep_state;    // 同 sleep_state 语义
    int8_t sum_avg_resp;       // 0~? -1 未知
    int8_t sum_avg_hr;         // 0~? -1 未知
    int8_t sum_turns;          // 次数
    int8_t sum_max_motion;     // 0~100
    int8_t sum_min_motion;     // 0~100
    int8_t sum_resp_stop;      // 次数
} radar_state_t;

// 函数声明
void radar_init(void);           // 初始化USART3与内部状态
void radar_process_data(void);   // 轮询调用: 从USART3缓冲读取并解析帧
radar_state_t radar_get_state(void); // 获取最新状态快照(值返回)

#endif

