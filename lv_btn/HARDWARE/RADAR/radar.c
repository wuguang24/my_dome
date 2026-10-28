/*
 * 模块名称: 雷达数据解析实现 (radar.c)
 * 功能说明:
 *   - 解析来自 USART3 的雷达协议帧，更新状态快照
 *   - 采用环形缓冲思路拼接帧，并做基本校验
 */
#include "radar.h"
#include "string.h"
#include <stdio.h>

/* 调试开关: 1=打印详细日志, 0=关闭 */
#define RADAR_DEBUG 1

// 雷达状态(全局快照): 提供给其他任务读取
static radar_state_t radar_state;

// 接收缓冲区: 暂存未完整解析的数据
// 雷达帧最大长度约为 9+数据长度，通常不超过64字节，256字节足够
#define RADAR_RX_BUF_SIZE 256
static uint8_t radar_rx_buffer[RADAR_RX_BUF_SIZE];
static size_t radar_rx_len = 0; // 当前有效长度

// 计算校验和
// 计算8位累加和(低8位)
static inline uint8_t u8_sum(const uint8_t* p, size_t n)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < n; ++i) sum += p[i];
    return (uint8_t)(sum & 0xFF);
}

// 解析有符号2字节数据
// 解析有符号2字节(最高位表示正负号)
// 根据手册: 首位为0表示为正，首位为1表示为负，剩下15位为幅值
static int16_t parse_signed_2b(uint8_t hi, uint8_t lo)
{
    uint16_t raw = ((uint16_t)hi << 8) | lo;
    // 最高位为 0 表示正，1 表示负；剩余15位为幅值
    int sign = (raw & 0x8000) ? -1 : +1;
    int16_t mag = (int16_t)(raw & 0x7FFF);
    return (int16_t)(sign * mag);
}

// 处理存在/运动数据
// 应用“存在/运动”类字段
static void apply_presence(uint8_t cmd, const uint8_t* d, size_t n)
{
    if (cmd == 0x01 && n >= 1) {
        radar_state.exist = (d[0] == 1) ? 1 : 0;
    } else if (cmd == 0x02 && n >= 1) {
        radar_state.movement = (int8_t)d[0];
    } else if (cmd == 0x03 && n >= 1) {
        radar_state.body_motion = (int8_t)d[0];
    } else if (cmd == 0x04 && n >= 2) {
        radar_state.distance_cm = (int16_t)(((uint16_t)d[0] << 8) | d[1]);
    } else if (cmd == 0x05 && n >= 6) {
        radar_state.pos_x_cm = parse_signed_2b(d[0], d[1]);
        radar_state.pos_y_cm = parse_signed_2b(d[2], d[3]);
        radar_state.pos_z_cm = parse_signed_2b(d[4], d[5]);
    }
}

// 处理呼吸数据
// 应用“呼吸”类字段
static void apply_respiration(uint8_t cmd, const uint8_t* d, size_t n)
{
    if (cmd == 0x01 && n >= 1) {
        radar_state.respiration_state = (int8_t)d[0];
    } else if (cmd == 0x02 && n >= 1) {
        radar_state.respiration = (int8_t)d[0];
    }
}

// 处理心率数据
// 应用“心率”类字段
static void apply_heart(uint8_t cmd, const uint8_t* d, size_t n)
{
    if (cmd == 0x02 && n >= 1) {
        radar_state.heart_rate = (int8_t)d[0]; if(radar_state.heart_rate > 0) radar_state.exist = 1;
    }
}

// 处理睡眠数据
// 应用“睡眠/十分钟综合”类字段
static void apply_sleep(uint8_t cmd, const uint8_t* d, size_t n)
{
    if (cmd == 0x01 && n >= 1) {
        radar_state.in_bed = (int8_t)d[0];
    } else if (cmd == 0x02 && n >= 1) {
        radar_state.sleep_state = (int8_t)d[0];
    } else if (cmd == 0x0C && n >= 8) {
        radar_state.sum_exist = (int8_t)d[0];
        radar_state.sum_sleep_state = (int8_t)d[1];
        radar_state.sum_avg_resp = (int8_t)d[2];
        radar_state.sum_avg_hr = (int8_t)d[3];
        radar_state.sum_turns = (int8_t)d[4];
        radar_state.sum_max_motion = (int8_t)d[5];
        radar_state.sum_min_motion = (int8_t)d[6];
        radar_state.sum_resp_stop = (int8_t)d[7];
    }
}

// 处理一帧数据
// 处理单帧: 校验 -> 分发不同 ctl 类别
static void handle_frame(const uint8_t* f, size_t n)
{
    // f 格式：53 59 Ctl Cmd LenH LenL Data.. Sum 54 43
    if (n < 9) return;
    uint8_t ctl = f[2];
    uint8_t cmd = f[3];
    uint16_t len = ((uint16_t)f[4] << 8) | f[5];
    if (6u + len + 3u > n) return;
    const uint8_t* data = &f[6];
    uint8_t sum = f[6 + len];
    // 校验（含 53,59）
    uint8_t calc = u8_sum(f, 6 + len);
    if (calc != sum) {
#if RADAR_DEBUG
        printf("[RADAR] checksum fail: calc=0x%02X, recv=0x%02X\r\n", calc, sum);
#endif
        return; // 校验失败
    }
#if RADAR_DEBUG
    printf("[RADAR] frame OK: ctl=0x%02X cmd=0x%02X len=%u\r\n", ctl, cmd, len);
#endif
    // 应用数据
    switch (ctl) {
        case 0x80: apply_presence(cmd, data, len); break;
        case 0x81: apply_respiration(cmd, data, len); break;
        case 0x84: apply_sleep(cmd, data, len); break;
        case 0x85: apply_heart(cmd, data, len); break;
        default: break;
    }
}

// 处理接收到的字节
// 累积原始字节流并尽可能解析出完整帧
static void feed_bytes(const uint8_t* p, size_t n)
{
    if (n == 0) return;
    // 追加到缓冲
    if (radar_rx_len + n > sizeof(radar_rx_buffer)) {
        // 缓冲区即将溢出，尝试找到最后一个帧头位置，保留从该位置开始的数据
        size_t keep_from = 0;
        for (size_t j = radar_rx_len; j > 0; j--) {
            if (j >= 2 && radar_rx_buffer[j-2] == 0x53 && radar_rx_buffer[j-1] == 0x59) {
                keep_from = j - 2;
                break;
            }
        }
        if (keep_from > 0) {
            memmove(radar_rx_buffer, radar_rx_buffer + keep_from, radar_rx_len - keep_from);
            radar_rx_len = radar_rx_len - keep_from;
        } else {
            // 没找到帧头，清空缓冲区
            radar_rx_len = 0;
        }
    }
    // 再次检查是否有足够空间
    if (radar_rx_len + n > sizeof(radar_rx_buffer)) {
        // 仍然不够，丢弃新数据
        return;
    }
    memcpy(radar_rx_buffer + radar_rx_len, p, n);
    radar_rx_len += n;

    // 尝试解析多帧
    size_t i = 0;
    while (i + 9 <= radar_rx_len) {
        // 寻找帧头 53 59
        if (!(radar_rx_buffer[i] == 0x53 && radar_rx_buffer[i + 1] == 0x59)) {
            i++;
            continue;
        }
        if (i + 7 > radar_rx_len) break; // 不够读长度
        uint16_t len = ((uint16_t)radar_rx_buffer[i + 4] << 8) | radar_rx_buffer[i + 5];
        size_t frame_len = 2 + 1 + 1 + 2 + len + 1 + 2; // hdr+ctl+cmd+len+data+sum+tail
        if (i + frame_len > radar_rx_len) break; // 待更多数据
        // 校验尾 54 43
        if (!(radar_rx_buffer[i + frame_len - 2] == 0x54 && radar_rx_buffer[i + frame_len - 1] == 0x43)) {
            // 不是合法尾，从下一个字节继续
            i++;
            continue;
        }
        handle_frame(&radar_rx_buffer[i], frame_len);
        i += frame_len;
    }
    // 收缩缓冲
    if (i > 0) {
        memmove(radar_rx_buffer, radar_rx_buffer + i, radar_rx_len - i);
        radar_rx_len -= i;
    }
}

// 初始化雷达
// 初始化雷达: 清空状态/缓冲并初始化 USART3
void radar_init(void)
{
    memset(&radar_state, 0, sizeof(radar_state));
    // 初始未知值
    radar_state.exist = -1;
    radar_state.movement = -1;
    radar_state.distance_cm = -1;
    radar_state.body_motion = -1;
    radar_state.pos_x_cm = 32767;
    radar_state.pos_y_cm = 32767;
    radar_state.pos_z_cm = 32767;
    radar_state.respiration = -1;
    radar_state.respiration_state = -1;
    radar_state.heart_rate = -1;
    radar_state.in_bed = -1;
    radar_state.sleep_state = -1;
    radar_state.sum_exist = -1;
    radar_state.sum_sleep_state = -1;
    radar_state.sum_avg_resp = -1;
    radar_state.sum_avg_hr = -1;
    radar_state.sum_turns = -1;
    radar_state.sum_max_motion = -1;
    radar_state.sum_min_motion = -1;
    radar_state.sum_resp_stop = -1;

    radar_rx_len = 0;
    memset(radar_rx_buffer, 0, sizeof(radar_rx_buffer));

    // 初始化USART3
    uart3_init(115200);
    printf("[RADAR] init OK, USART3@115200\r\n");
}

// 处理雷达数据
// 处理从 USART3 收到的数据(轮询)
// 使用临界区保护，避免在处理过程中丢失数据
void radar_process_data(void)
{
    size_t len = USART3_RX_STA & 0X3FFF;
    if (len > 0) {
        // 禁用USART3中断，防止数据竞争
        HAL_NVIC_DisableIRQ(USART3_IRQn);
        
        // 直接从USART3缓冲区处理数据，避免额外复制
        size_t copy_len = USART3_RX_STA & 0X3FFF;
        if (copy_len > 0) {
            feed_bytes(USART3_RX_BUF, copy_len);
            USART3_RX_STA = 0; // 清空接收状态
        }
        
        // 重新使能USART3中断
        HAL_NVIC_EnableIRQ(USART3_IRQn);
    }
}

// 获取雷达状态
// 获取当前雷达状态(值返回)
radar_state_t radar_get_state(void)
{
    return radar_state;
}

