/*
 * 模块名称: 边缘应急探测算法 (edge_alarm)
 * 功能概述:
 *   - 基于雷达本地数据在设备端进行应急判定：呼吸暂停、心率异常、异常挣扎、长时间无人
 *   - 计算生命可能性评分 LifeScore(0..100) 与近5秒距离趋势(靠近/远离/稳定)
 *   - 输出结构 edge_alarm_out_t 供 UI、蜂鸣器、LED 与离线事件模块使用
 * 数据结构:
 *   - edge_alarm_out_t: 本次判定的告警等级/标志、LifeScore、趋势
 *   - edge_alarm_cfg_t: 本地阈值参数(可后续支持从24CXX加载/保存)
 * 外部接口:
 *   - edge_alarm_init(): 初始化默认阈值
 *   - edge_alarm_cfg(): 读取/修改当前阈值指针
 *   - edge_alarm_update(r, now_ms, out): 基于雷达状态与时间戳进行一次评估
 */
#ifndef __EDGE_ALARM_H
#define __EDGE_ALARM_H

#include "sys.h"
#include "radar.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 趋势定义: -1 远离 0 稳定 1 靠近 */
typedef struct {
    uint8_t apnea_level;      /* 0 无, 1 预警, 2 报警, 3 危急 */
    uint8_t hr_alarm_level;   /* 0 无, 1 低, 2 高 */
    uint8_t struggle;         /* 0/1 */
    uint8_t away_too_long;    /* 0/1 */
    uint8_t life_score;       /* 0..100 */
    int8_t  trend;            /* -1/0/1 */
} edge_alarm_out_t;

typedef struct {
    uint16_t apnea_warn_ms;       /* 10000 */
    uint16_t apnea_alarm_ms;      /* 20000 */
    uint16_t apnea_critical_ms;   /* 30000 */
    uint8_t  hr_low;              /* 45 */
    uint8_t  hr_high;             /* 120 */
    uint8_t  struggle_motion;     /* 80 */
    uint16_t struggle_ms;         /* 5000 */
    uint16_t away_timeout_ms;     /* 60000 */
} edge_alarm_cfg_t;

/* 初始化边缘算法参数(默认阈值) */
void edge_alarm_init(void);
/* 访问当前阈值配置(可修改后生效) */
edge_alarm_cfg_t* edge_alarm_cfg(void);
/*
 * 进行一次判定与打分
 * 参数:
 *   r       : 雷达状态输入(存在/体动/距离/呼吸/心率等)
 *   now_ms  : 当前毫秒时间(用于持续时长判断)
 *   out     : 输出告警/评分/趋势
 */
void edge_alarm_update(const radar_state_t* r, uint32_t now_ms, edge_alarm_out_t* out);

#ifdef __cplusplus
}
#endif

#endif
