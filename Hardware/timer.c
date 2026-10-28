#include "timer.h"
#include "stm32f10x_tim.h"

// 定义变量
// flag_100ms / flag_1s：由 TIM3 中断置位的“软定时”标志。
// 主循环中检测到标志后执行对应周期任务，并把标志清零。
// 这样可以把周期性逻辑放在主循环里做，避免在中断里做耗时操作。
volatile uint8_t flag_100ms = 0;
volatile uint8_t flag_1s = 0;
// rain_cd：雨天/降雨相关逻辑的冷却倒计时（单位：1s）。
// 本文件只负责每秒递减，具体使用场景由业务层决定。
volatile uint16_t rain_cd = 0;
// fire_flag：火灾标志位（业务层设置/读取，本文件不主动改变）
volatile uint8_t fire_flag = 0;  // 火灾标志

// TIM3初始化
// 目标：产生 10ms 的基础节拍，在中断里进一步累加得到 100ms / 1s 标志。
// 说明：STM32F103 常见系统时钟为 72MHz。
// - 预分频 7200-1 => 计数时钟约 72MHz/7200 = 10kHz（周期 0.1ms）
// - 自动重装 100-1 => 100 个计数 => 10ms 产生一次更新中断
void TIM3_Init(void)
{
    TIM_TimeBaseInitTypeDef t;
    NVIC_InitTypeDef n;
    
    // 开时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 配置：分7200倍，数100次=10ms（见上方说明）
    t.TIM_Period = 100 - 1;      
    t.TIM_Prescaler = 7200 - 1;  
    t.TIM_ClockDivision = 0;
    t.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &t);
    
    // 使能中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // 配中断
    n.NVIC_IRQChannel = TIM3_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 0;
    n.NVIC_IRQChannelSubPriority = 3;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);
    
    // 启动
    TIM_Cmd(TIM3, ENABLE);
}

// TIM3中断
// 注意：中断里只做“计数 + 置标志”，避免影响系统实时性。
void TIM3_IRQHandler(void)
{
    static uint8_t cnt_10ms = 0;
    static uint8_t cnt_100ms = 0;
    
    if(TIM_GetITStatus(TIM3, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        cnt_10ms++;
        cnt_100ms++;
        
        // 10 次 10ms = 100ms：置位 100ms 标志供主循环消费
        if(cnt_10ms >= 10)
        {
            cnt_10ms = 0;
            flag_100ms = 1;
        }
        
        // 100 次 10ms = 1s：置位 1s 标志；同时维护 1s 级别的倒计时
        if(cnt_100ms >= 100)
        {
            cnt_100ms = 0;
            flag_1s = 1;
            
            // 雨倒计时减1
            if(rain_cd > 0) rain_cd--;
        }
    }
}
