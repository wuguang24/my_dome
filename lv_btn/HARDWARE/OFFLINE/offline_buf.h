/*
 * 模块名称: 离线事件环形缓冲 (offline_buf)
 * 功能概述:
 *   - 将本地应急事件(暂停/心率异常/挣扎/无人等)写入 24C02 EEPROM 形成环形队列
 *   - 设备离线时持续缓存，上线后由 Cloud_FlushQueue() 统一补传
 * 数据结构:
 *   - offline_event_t: 定长12字节事件记录(时间戳/状态/事件码/CRC)
 * 外部接口:
 *   - offline_init(): 初始化 EEPROM 与指针(添加魔数/校验)
 *   - offline_push(e): 写入一条事件(自动覆盖最旧)
 *   - offline_has(): 判断是否有待取事件
 *   - offline_pop(e): 读取并弹出最旧事件
 */
#ifndef __OFFLINE_BUF_H
#define __OFFLINE_BUF_H

#include "sys.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 事件记录（12字节，适配24C02容量） */
typedef struct {
    uint32_t ts;       /* ms tick 或秒 */
    uint8_t  exist;    /* 0/1 */
    uint8_t  hr;       /* 0..255 */
    uint8_t  resp;     /* 0..255 */
    uint8_t  motion;   /* 0..100 */
    uint16_t distance; /* cm */
    uint8_t  code;     /* 事件码 */
    uint8_t  crc;      /* 简单校验 */
} offline_event_t; /* size=12 */

/* 初始化 EEPROM 环形缓冲(写入魔数/恢复指针) */
void offline_init(void);
/* 写入一条事件(覆盖策略: 满则前移 tail) */
bool offline_push(const offline_event_t* e);
/* 是否存在待上报事件 */
bool offline_has(void);
/* 读取并弹出一条最旧事件(校验失败则丢弃该条) */
bool offline_pop(offline_event_t* e);

#ifdef __cplusplus
}
#endif

#endif
