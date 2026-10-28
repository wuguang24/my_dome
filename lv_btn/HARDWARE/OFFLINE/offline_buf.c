/*
 * 模块名称: 离线事件环形缓冲实现 (offline_buf.c)
 * 功能说明:
 *   - 在 24C02 上维护固定长度事件记录的环形队列，实现断网期间本地缓存
 *   - 通过魔数与指针(head/tail)持久化，掉电不丢失
 */
#include "offline_buf.h"  // 事件结构与对外 API
#include "24cxx.h"       // EEPROM 读写接口
#include <string.h>        // memcpy 等

/*
 * 存储布局:
 *  [0x00] 魔数 A(0xA5)
 *  [0x01] 魔数 B(0x5A)
 *  [0x02] 保留
 *  [0x03] 保留
 *  [0x04] head(写入指针)
 *  [0x05] tail(读取指针)
 *  [0x10..] 固定长度记录区(每条12字节)
 */
#define OFF_MAGIC_A   0xA5
#define OFF_MAGIC_B   0x5A
#define OFF_HDR_ADDR  0x00
#define OFF_HEAD_ADDR 0x04 /* 1B */
#define OFF_TAIL_ADDR 0x05 /* 1B */
#define OFF_BASE_ADDR 0x10
#define OFF_SLOT_SIZE 12

/* 以 AT24C02 为例：总 256 字节。可用槽数： (256-0x10)/12 = 20 */
#define OFF_CAPACITY  ((uint8_t)((((EE_TYPE)+1)-OFF_BASE_ADDR)/OFF_SLOT_SIZE))

static uint8_t s_head = 0; /* 写入位置(索引 0..OFF_CAPACITY-1) */
static uint8_t s_tail = 0; /* 读取位置(索引 0..OFF_CAPACITY-1) */

/*
 * 函数: crc8_xor
 * 作用: 简单异或校验，计算 0..len-1 的按位异或
 */
static uint8_t crc8_xor(const uint8_t* p, uint8_t len)
{
    uint8_t c = 0;
    for(uint8_t i=0;i<len;i++) c ^= p[i];
    return c;
}

/* 计算第 idx 条记录的物理地址 */
static uint16_t slot_addr(uint8_t idx)
{
    return (uint16_t)(OFF_BASE_ADDR + (uint16_t)idx * OFF_SLOT_SIZE);
}

/* 持久化 head/tail 指针到 EEPROM */
static void save_ptrs(void)
{
    AT24CXX_WriteOneByte(OFF_HEAD_ADDR, s_head);
    AT24CXX_WriteOneByte(OFF_TAIL_ADDR, s_tail);
}

/* 初始化环形缓冲：校验/写入魔数，恢复/纠正指针 */
void offline_init(void)
{
    AT24CXX_Init();
    if(!AT24CXX_Check()) {
        /* EEPROM 不可用则将指针置零（仍允许运行但不会持久化） */
        s_head = s_tail = 0;
        return;
    }
    /* 读取魔数 */
    uint8_t a = AT24CXX_ReadOneByte(OFF_HDR_ADDR+0);
    uint8_t b = AT24CXX_ReadOneByte(OFF_HDR_ADDR+1);
    if(a != OFF_MAGIC_A || b != OFF_MAGIC_B) {
        /* 初始化头 */
        AT24CXX_WriteOneByte(OFF_HDR_ADDR+0, OFF_MAGIC_A);
        AT24CXX_WriteOneByte(OFF_HDR_ADDR+1, OFF_MAGIC_B);
        AT24CXX_WriteOneByte(OFF_HDR_ADDR+2, 0);
        AT24CXX_WriteOneByte(OFF_HDR_ADDR+3, 0);
        s_head = s_tail = 0;
        save_ptrs();
    } else {
        s_head = AT24CXX_ReadOneByte(OFF_HEAD_ADDR);
        s_tail = AT24CXX_ReadOneByte(OFF_TAIL_ADDR);
        if(s_head >= OFF_CAPACITY || s_tail >= OFF_CAPACITY) {
            s_head = s_tail = 0;
            save_ptrs();
        }
    }
}

/* 写入一条事件，满时覆盖最旧并移动 tail */
bool offline_push(const offline_event_t* e)
{
    if(!e) return false;
    uint8_t buf[OFF_SLOT_SIZE];
    memcpy(buf, e, OFF_SLOT_SIZE);
    buf[OFF_SLOT_SIZE-1] = crc8_xor(buf, OFF_SLOT_SIZE-1);
    uint16_t addr = slot_addr(s_head);
    AT24CXX_Write(addr, buf, OFF_SLOT_SIZE);
    uint8_t next = (uint8_t)((s_head + 1) % OFF_CAPACITY);
    s_head = next;
    if(s_head == s_tail) {
        /* 覆盖最旧 */
        s_tail = (uint8_t)((s_tail + 1) % OFF_CAPACITY);
    }
    save_ptrs();
    return true;
}

/* 是否存在待取事件 */
bool offline_has(void)
{
    return s_head != s_tail;
}

/* 读取并弹出一条最旧事件；CRC 错误则丢弃该条 */
bool offline_pop(offline_event_t* e)
{
    if(!e) return false;
    if(s_head == s_tail) return false;
    uint8_t buf[OFF_SLOT_SIZE];
    AT24CXX_Read(slot_addr(s_tail), buf, OFF_SLOT_SIZE);
    uint8_t c = crc8_xor(buf, OFF_SLOT_SIZE-1);
    if(c != buf[OFF_SLOT_SIZE-1]) {
        /* 校验失败，丢弃该条 */
        s_tail = (uint8_t)((s_tail + 1) % OFF_CAPACITY);
        save_ptrs();
        return false;
    }
    memcpy(e, buf, OFF_SLOT_SIZE);
    s_tail = (uint8_t)((s_tail + 1) % OFF_CAPACITY);
    save_ptrs();
    return true;
}
