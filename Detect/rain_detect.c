#include "rain_detect.h"

// 雨量级别
uint8_t rain_lvl(uint16_t adc)
{
    if (adc > RAIN_BIG) return 2;      // 大雨
    else if (adc > RAIN_SML) return 1; // 小雨
    else return 0;                     // 无雨
}

// 雨量控制（移除未使用的变量）
void rain_ctrl(uint8_t lvl, uint8_t fire, uint32_t time, uint8_t* win_closed, 
               uint32_t* timer, uint16_t* rain_cd, uint8_t* rain_on)
{
    // 火灾时不处理雨量
    if (fire) 
    {
        *win_closed = 0;
        *rain_on = 0;
        return;
    }
    
    // 大雨：关窗
    if (lvl == 2)
    {
        if (!(*win_closed))
        {
            *win_closed = 1;          // 标记窗户已关闭
            *timer = time;            // 记录起始时间
            *rain_cd = RAIN_WAIT;     // 180秒
            *rain_on = 1;
        }
        else
        {
            // 更新倒计时
            uint32_t elapsed = (time - (*timer)) / 10; // 转换为秒
            if (elapsed < *rain_cd)
            {
                *rain_cd = RAIN_WAIT - elapsed;
            }
            else
            {
                *rain_cd = 0;
            }
        }
    }
    // 小雨或无雨
    else
    {
        if (*win_closed)
        {
            // 更新倒计时
            uint32_t elapsed = (time - (*timer)) / 10; // 转换为秒
            if (elapsed >= RAIN_WAIT)
            {
                *win_closed = 0;  // 开窗
                *rain_cd = 0;
                *rain_on = 0;
            }
            else
            {
                *rain_cd = RAIN_WAIT - elapsed;
                *rain_on = 1;
            }
        }
        else
        {
            *rain_on = 0;
        }
    }
}
