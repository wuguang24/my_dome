/*
 * 模块名称: 边缘应急探测算法实现 (edge_alarm.c)
 * 功能说明:
 *   - 根据雷达采集的局部状态，进行本地实时判定：
 *     呼吸暂停分级/心率异常/挣扎/无人超时
 *   - 计算生命可能性评分 LifeScore(0..100)
 *   - 计算近5秒距离趋势(靠近/远离/稳定)，用于 SAR 引导
 * 设计要点:
 *   - 使用简单阈值 + 时间窗累计，计算复杂度低，利于 F4 实时
 *   - 距离趋势通过 32 点(≈5s) 的前后半段均值比较来估计
 */
#include "edge_alarm.h"   // 对应头文件，包含配置结构与输出结构定义
#include <string.h>        // 使用 memset 等标准库函数

static edge_alarm_cfg_t g_cfg; // 全局配置(默认阈值，后续可从24CXX加载)

/*
 * 函数名称: edge_alarm_init
 * 功能描述: 初始化默认阈值配置
 * 参    数: 无
 * 返 回 值: 无
 */
void edge_alarm_init(void)
{
    /* 默认阈值，可后续支持从24CXX加载 */
    g_cfg.apnea_warn_ms     = 10000;
    g_cfg.apnea_alarm_ms    = 20000;
    g_cfg.apnea_critical_ms = 30000;
    g_cfg.hr_low            = 45;
    g_cfg.hr_high           = 120;
    g_cfg.struggle_motion   = 80;
    g_cfg.struggle_ms       = 5000;
    g_cfg.away_timeout_ms   = 60000;
}

/*
 * 函数名称: edge_alarm_cfg
 * 功能描述: 返回配置结构指针，便于外部读取/修改阈值
 * 参    数: 无
 * 返 回 值: edge_alarm_cfg_t* 配置指针
 */
edge_alarm_cfg_t* edge_alarm_cfg(void)
{
    return &g_cfg;
}

/*
 * 内部状态结构: 记录持续计时与距离历史，用于趋势估计
 */
typedef struct {
    uint32_t apnea_start_ms;      // 呼吸暂停开始时间戳(毫秒)
    uint32_t struggle_start_ms;   // 挣扎开始时间戳(毫秒)
    uint32_t away_start_ms;       // 无人开始时间戳(毫秒)
    uint16_t dist_hist[32];       // 距离历史(32 点)，约 5s 窗口(取样周期~160ms)
    uint8_t  dist_idx;            // 环形缓冲写入索引 0..31
    uint8_t  dist_cnt;            // 已累计的有效点个数 0..32
    uint32_t last_valid_ms;       // 最近一次有有效生命体征的时间戳(可用于健康心跳)
} edge_state_t;

static edge_state_t s; // 模块内部静态状态

/*
 * 函数名称: calc_trend
 * 功能描述: 根据 32 点距离历史的前/后半段均值对比，估计趋势
 * 参    数: 无(使用内部 s)
 * 返 回 值: -1 远离, 0 稳定, 1 靠近
 */
static int8_t calc_trend(void)
{
    if(s.dist_cnt < 4) return 0; // 点数过少，无法判断
    /* 简单线性：比较后半段平均与前半段平均 */
    uint32_t n = s.dist_cnt;            // 当前累计点数
    uint32_t half = n/2;                // 前半/后半分界
    uint32_t sum1 = 0, sum2 = 0;        // 前半/后半的距离和
    // 前半段累加
    for(uint32_t i=0;i<half;i++) sum1 += s.dist_hist[(s.dist_idx + 32 - n + i) & 31u];
    // 后半段累加
    for(uint32_t i=half;i<n;i++) sum2 += s.dist_hist[(s.dist_idx + 32 - n + i) & 31u];
    float avg1 = (float)sum1/(float)half;        // 前半均值
    float avg2 = (float)sum2/(float)(n-half);    // 后半均值
    if(avg2 + 5.0f < avg1) return 1;   /* 距离变小 -> 靠近 (阈值5cm) */
    if(avg2 > avg1 + 5.0f) return -1;  /* 距离变大 -> 远离 (阈值5cm) */
    return 0;                          // 变化不明显 -> 稳定
}

/*
 * 函数名称: edge_alarm_update
 * 功能描述: 核心评估流程：更新历史、判定四类事件、计算评分与趋势
 * 参    数: r      雷达状态输入(含存在/距离/体动/HR/Resp 等)
 *           now_ms 当前毫秒时间(用于持续判定)
 *           out    输出结果指针
 * 返 回 值: 无(结果写入 out)
 */
void edge_alarm_update(const radar_state_t* r, uint32_t now_ms, edge_alarm_out_t* out)
{
    if(!r || !out) return;                 // 判空保护
    memset(out, 0, sizeof(*out));          // 清空输出结构

    /* 更新距离历史(无效距离置为 0xFFFF 以弱化趋势影响) */
    uint16_t d = (r->distance_cm >= 0) ? (uint16_t)r->distance_cm : 65535u;
    s.dist_hist[s.dist_idx] = d;                   // 写入当前点
    s.dist_idx = (s.dist_idx + 1) & 31u;           // 环形+1
    if(s.dist_cnt < 32) s.dist_cnt++;              // 增加有效点计数

    /* 呼吸暂停判定：呼吸为0 或 respiration_state==4(无) 计时 */
    bool apnea_now = (r->respiration == 0) || (r->respiration_state == 4);
    if(apnea_now) {
        if(s.apnea_start_ms == 0) s.apnea_start_ms = now_ms; // 记录开始时刻
        uint32_t dur = now_ms - s.apnea_start_ms;            // 持续时长
        if(dur >= g_cfg.apnea_critical_ms) out->apnea_level = 3;     // 危急
        else if(dur >= g_cfg.apnea_alarm_ms) out->apnea_level = 2;   // 报警
        else if(dur >= g_cfg.apnea_warn_ms) out->apnea_level = 1;    // 预警
        else out->apnea_level = 0;                                   // 未达阈值
    } else {
        s.apnea_start_ms = 0;         // 恢复呼吸，清零计时
        out->apnea_level = 0;         // 清除暂停等级
    }

    /* 心率异常（低/高） */
    if(r->heart_rate >= 0) {
        if(r->heart_rate < g_cfg.hr_low) out->hr_alarm_level = 1;  /* 低 */
        else if(r->heart_rate > g_cfg.hr_high) out->hr_alarm_level = 2; /* 高 */
        else out->hr_alarm_level = 0;  /* 正常 */
    }

    /* 挣扎：体动幅度超过阈值并且持续一定时间 */
    bool struggle_now = (r->body_motion >= 0) && ((uint8_t)r->body_motion >= g_cfg.struggle_motion);
    if(struggle_now) {
        if(s.struggle_start_ms == 0) s.struggle_start_ms = now_ms; // 首次超过阈值，开始计时
        if(now_ms - s.struggle_start_ms >= g_cfg.struggle_ms) out->struggle = 1; // 持续满足 -> 标记挣扎
    } else {
        s.struggle_start_ms = 0;      // 退出挣扎区，清零计时
    }

    /* 离床/无人超时：exist 为 0 持续达到阈值 */
    bool away_now = (r->exist == 0);
    if(away_now) {
        if(s.away_start_ms == 0) s.away_start_ms = now_ms; // 记录无人开始时刻
        if(now_ms - s.away_start_ms >= g_cfg.away_timeout_ms) out->away_too_long = 1; // 达到无人超时
    } else {
        s.away_start_ms = 0;          // 恢复有人，清零计时
    }

    /* 生命评分：存在/有效 HR/Resp/体动与近距离加分，范围 0..100 */
    float score = 0.0f;
    if(r->exist == 1) score += 40.0f;                  // 存在权重
    if(r->heart_rate >= 0) score += 30.0f;             // HR 有效权重
    if(r->respiration >= 0) score += 20.0f;            // Resp 有效权重
    if(r->body_motion >= 0) score += (float)r->body_motion * 0.1f; /* 体动 0..100 -> 0..10 */
    if(r->distance_cm >= 0) {
        float dist = (float)r->distance_cm;
        if(dist < 100.0f) score += 10.0f;          /* <1m 加成 */
        else if(dist < 200.0f) score += 5.0f;      /* <2m 加成 */
    }
    if(score < 0.0f) score = 0.0f; if(score > 100.0f) score = 100.0f; // 限幅
    out->life_score = (uint8_t)score;  // 转为 0..100 的整数

    /* 趋势：-1 远离 / 0 稳定 / 1 靠近 */
    out->trend = calc_trend();

    /* 有效性时间戳（可用于健康心跳） */
    if(r->exist == 1 || r->heart_rate >= 0 || r->respiration >= 0) s.last_valid_ms = now_ms;
}
