#include "wind_detect.h"

// 检查风速等级
uint8_t wind_chk(float wind)
{
    if (wind >= WIND_HIGH) return 2;      // 大风，需要关窗
    else if (wind < WIND_LOW) return 0;   // 小风，可以开窗
    else return 1;                        // 中风，保持状态
}

// 风速控制
void wind_ctrl(float wind, uint8_t fire, uint8_t rain, 
               uint8_t* wind_closed, uint16_t* wind_cd, 
               uint8_t* wind_wait, uint16_t* wind_wait_cd)
{
    // 火灾或大雨时，风速控制无效
    if (fire || rain) 
    {
        *wind_closed = 0;
        *wind_cd = 0;
        *wind_wait = 0;
        *wind_wait_cd = 0;
        return;
    }
    
    uint8_t lvl = wind_chk(wind);
    
    // 大风：关窗，60秒倒计时
    if (lvl == 2)  // 风速 ≥ 8m/s
    {
        if (!(*wind_closed))
        {
            *wind_closed = 1;        // 标记窗户已关闭
            *wind_cd = WIND_CD_60;   // 60秒倒计时
            *wind_wait = 0;
            *wind_wait_cd = 0;
        }
        
        // 倒计时中持续大风：重置倒计时
        if (*wind_cd == 0 && *wind_closed)
        {
            *wind_cd = WIND_CD_60;  // 重置60秒
            *wind_wait = 0;         // 清除等待状态
            *wind_wait_cd = 0;
        }
    }
    // 60秒倒计时结束，检测到小风 <5m/s
    else if (*wind_cd == 0 && *wind_closed)
    {
        if (lvl == 0)  // 小风 < 5m/s
        {
            // 进入等待状态
            if (!(*wind_wait))  // 避免重复设置
            {
                *wind_wait = 1;
                *wind_wait_cd = WIND_WAIT_30;  // 设置30秒等待
            }
            // 保持关窗状态
            *wind_closed = 1;
        }
        else  // 中风：5-8m/s
        {
            *wind_cd = WIND_CD_60;  // 重新开始60秒倒计时
            *wind_wait = 0;         // 清除等待状态
            *wind_wait_cd = 0;
        }
    }
    
    // 等待状态处理
    if (*wind_wait)
    {
        if (*wind_wait_cd > 0)
        {
            // 等待期间检测风速
            if (lvl >= 1)  // 中风或大风
            {
                // 风速≥5m/s → 取消等待
                *wind_wait = 0;
                *wind_wait_cd = 0;
                *wind_closed = 1;  // 保持关窗
                *wind_cd = WIND_CD_60;  // 重新开始60秒倒计时
            }
        }
        else if (*wind_wait_cd == 0)  // 等待结束
        {
            // 30秒等待结束 → 开窗
            *wind_wait = 0;
            *wind_closed = 0;  // 开窗
        }
    }
    
    // 小风且不在等待状态 → 直接开窗
    if (lvl == 0 && !(*wind_wait) && !(*wind_closed) && *wind_cd == 0)
    {
        // 正常状态小风开窗
        *wind_closed = 0;
    }
}
