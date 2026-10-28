/*
 * 模块名称: 蜂鸣器驱动实现 (beep.c)
 * 功能说明:
 *   - 初始化 PG13 为推挽输出，默认关闭
 *   - 提供设置开关与节奏鸣叫接口(使用 FreeRTOS 延时)
 */
#include "beep.h"      // 蜂鸣器控制头文件
#include "delay.h"     // 延时接口（含os_delay_ms封装）
#include "FreeRTOS.h"  // FreeRTOS 核心
#include "task.h"      // 任务API（vTaskDelay等）

// 蜂鸣器初始化: 配置 PG13 为推挽输出，默认置低(静音)
void BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0}; // GPIO 初始化结构体
    
    BEEP_GPIO_CLK_ENABLE();                 // 使能 GPIOG 时钟（必需先开时钟）
    
    GPIO_InitStruct.Pin = BEEP_GPIO_PIN;    // PG13
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;     // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // 低速足够（降低EMI）
    HAL_GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStruct); // 初始化引脚
    
    // 初始化为关闭状态
    BEEP_OFF();
}

// 设置蜂鸣器状态: state=1 开 / state=0 关
void BEEP_Set(uint8_t state)
{
    if(state)
    {
        BEEP_ON();   // 置高电平 -> 鸣叫
    }
    else
    {
        BEEP_OFF();  // 置低电平 -> 静音
    }
}

// 蜂鸣器鸣叫: times 次，单次 on/off 时长使用 FreeRTOS 延时（若未启调度，可由delay封装适配）
void BEEP_Beep(uint16_t times, uint16_t on_time_ms, uint16_t off_time_ms)
{
    uint16_t i; // 计数器
    
    for(i = 0; i < times; i++)
    {
        BEEP_ON();                                              // 打开
        vTaskDelay(pdMS_TO_TICKS(on_time_ms));                  // 保持 on（RTOS节拍延时）
        BEEP_OFF();                                             // 关闭
        if(i < times - 1) // 最后一次不需要延时
        {
            vTaskDelay(pdMS_TO_TICKS(off_time_ms));             // 仅在非最后一次插入 off 延时
        }
    }
}

