#include "hcsr04.h"
#include "board.h"

/* 带安装偏移的舵机角度设置，并限制在0-180度 */
uint8_t servo_angle_offset(uint8_t base_angle)
{
    int16_t a = (int16_t)base_angle + SERVO_MOUNT_OFFSET;
    if (a < 0) a = 0;
    if (a > 180) a = 180;
    return (uint8_t)a;
}

/* 调试计数器 */
static uint32_t g_hcsr04_debug_cnt = 0;

/* 读取超声波距离（阻塞式）
 * 返回值：距离（cm），失败返回-1
 * 修复：
 *   1. 使用tick_ms做超时计时（不受SysTick溢出影响）
 *   2. 发Trig前检查Echo是否为LOW（检测接线/硬件故障）
 *   3. 用tick_ms+SysTick->VAL做高精度Echo脉宽测量（处理多次溢出）
 *   4. Trig脉宽从12us加到20us提高可靠性
 */
float HCSR04_ReadDistanceCm(void)
{
    uint32_t timeout_start;
    float distance;
    uint32_t echo_time_us;
    uint32_t start_tick, end_tick, start_val, end_val;

    g_hcsr04_debug_cnt++;

    /* 0. 诊断：检查Echo引脚空闲状态（应该为LOW） */
    {
        uint32_t echo_idle = DL_GPIO_readPins(HCSR04_PORT, HCSR04_Echo_PIN);
        if (echo_idle != 0) {
            /* Echo空闲时为HIGH，说明接线可能有问题或者模块损坏 */
            if (g_hcsr04_debug_cnt <= 3 || g_hcsr04_debug_cnt % 50 == 0) {
                printf("[HCSR04] WARN: Echo is HIGH before Trig! (stuck pin or bad wiring)\r\n");
            }
            /* 尝试等待Echo变LOW（最多5ms） */
            timeout_start = (uint32_t)tick_ms;
            while (DL_GPIO_readPins(HCSR04_PORT, HCSR04_Echo_PIN) != 0) {
                if ((uint32_t)(tick_ms - timeout_start) >= 5U) {
                    if (g_hcsr04_debug_cnt % 50 == 0) {
                        printf("[HCSR04] Echo stuck HIGH, cannot measure\r\n");
                    }
                    return -1.0f;
                }
            }
        }
    }

    /* 1. 发送触发脉冲 - 20us（HC-SR04要求最低10us，加宽提高可靠性） */
    DL_GPIO_setPins(HCSR04_PORT, HCSR04_Trig_PIN);
    delay_us(20);
    DL_GPIO_clearPins(HCSR04_PORT, HCSR04_Trig_PIN);

    /* 2. 等待Echo引脚变为高电平（使用tick_ms超时，不受delay_us精度影响） */
    timeout_start = (uint32_t)tick_ms;
    while (DL_GPIO_readPins(HCSR04_PORT, HCSR04_Echo_PIN) == 0)
    {
        if ((uint32_t)(tick_ms - timeout_start) >= 30U) /* 30ms超时 */
        {
            if (g_hcsr04_debug_cnt % 10 == 0) {
                printf("[HCSR04] Timeout waiting for Echo HIGH\r\n");
            }
            return -1.0f;
        }
    }

    /* 3. 记录高电平开始时间（tick_ms + SysTick->VAL精细值） */
    start_tick = (uint32_t)tick_ms;
    start_val  = (uint32_t)(SysTick->VAL);

    /* 4. 等待Echo引脚变为低电平 */
    timeout_start = (uint32_t)tick_ms;
    while (DL_GPIO_readPins(HCSR04_PORT, HCSR04_Echo_PIN) != 0)
    {
        if ((uint32_t)(tick_ms - timeout_start) >= 40U) /* 40ms超时（比HC-SR04最大23ms大） */
        {
            if (g_hcsr04_debug_cnt % 10 == 0) {
                printf("[HCSR04] Timeout waiting for Echo LOW (after %lums)\r\n",
                       (unsigned long)(tick_ms - timeout_start));
            }
            return -1.0f;
        }
    }

    /* 5. 记录高电平结束时间 */
    end_tick = (uint32_t)tick_ms;
    end_val  = (uint32_t)(SysTick->VAL);

    /* 6. 计算Echo高电平持续时间（tick_ms处理多次溢出，VAL处理亚毫秒精度）
     * 总微秒 = (end_tick - start_tick) * 1000 + (start_val - end_val) / 80
     * start_val - end_val 用有符号算术，VAL翻转时为负值，由ms部分补偿
     */
    {
        uint32_t ms_diff = end_tick - start_tick;
        int32_t fine_us = ((int32_t)start_val - (int32_t)end_val) / (int32_t)(SysTickFre / 1000000U);
        if (fine_us >= 0) {
            echo_time_us = ms_diff * 1000U + (uint32_t)fine_us;
        } else {
            echo_time_us = ms_diff * 1000U - (uint32_t)(-fine_us);
        }
    }

    /* 7. 计算距离（厘米） */
    distance = (float)echo_time_us * 34000.0f / 2000000.0f;

    /* 每10次打印一次调试信息 */
    if (g_hcsr04_debug_cnt % 10 == 0) {
        printf("[HCSR04] echo_time=%lu us, distance=%.1f cm\r\n",
               (unsigned long)echo_time_us, (double)distance);
    }

    /* 8. 检查有效范围 */
    if (distance > 1.0f && distance < 400.0f)
    {
        return distance;
    }

    if (g_hcsr04_debug_cnt % 10 == 0) {
        printf("[HCSR04] Out of range: %.1f cm\r\n", (double)distance);
    }

    return -1.0f;
}

/* 异步测距接口空实现（当前使用阻塞式测距，异步接口未被实现，提供空函数避免链接错误） */
void HCSR04_AsyncStart(uint32_t now_ms) { (void)now_ms; }
int HCSR04_AsyncPoll(float *out_cm, uint32_t now_ms) { (void)out_cm; (void)now_ms; return 0; }
void HCSR04_AsyncOnEchoEdge(uint8_t level_high, uint32_t tick_val) { (void)level_high; (void)tick_val; }
uint8_t HCSR04_AsyncIsBusy(void) { return 0; }
uint8_t HCSR04_AsyncGetState(void) { return 0; }
int HCSR04_AsyncGetFailCode(void) { return 0; }
uint8_t HCSR04_AsyncGetFailStreak(void) { return 0; }
void HCSR04_ForceReset(void) {}
