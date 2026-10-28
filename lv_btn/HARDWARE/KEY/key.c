/*
 * 模块名称: 按键输入驱动实现 (key.c)
 * 功能说明:
 *   - 初始化 KEY0(PC0)/WK_UP(PA0) 为下拉输入
 *   - 提供 KEY_Scan(): 去抖扫描，支持非连按/连按两种模式
 */
#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F407ZGT6开发板
//按键输入驱动代码	   
//版本：V1.0									  
//////////////////////////////////////////////////////////////////////////////////

// 按键初始化函数: 使能时钟并配置 PC0/PA0 为输入下拉
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;  // GPIO 初始化结构体
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           // 开启 GPIOA 时钟(用于 WK_UP PA0)
    __HAL_RCC_GPIOC_CLK_ENABLE();           // 开启 GPIOC 时钟(用于 KEY0 PC0)

    GPIO_Initure.Pin=GPIO_PIN_0;            // PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      // 输入模式
    GPIO_Initure.Pull=GPIO_PULLDOWN;        // 下拉电阻: 未按下为低电平
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     // 高速(对输入影响不大)
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);     // 初始化 PA0
    
    GPIO_Initure.Pin=GPIO_PIN_0;            // PC0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      // 输入模式
    GPIO_Initure.Pull=GPIO_PULLDOWN;        // 下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     // 高速
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);     // 初始化 PC0
}

// 按键处理函数
// 返回按键值
// mode:0 不支持连续按; 1 支持连续按
// 返回: 0 无; 1 KEY0; 2 WK_UP
// 注意: 响应优先级 KEY0 > WK_UP
// 注意: 此函数使用软件去抖，需要在任务循环中定期调用（建议20ms周期）
u8 KEY_Scan(u8 mode)
{
    static u8 key_up = 1;           // 按键松开标志(去抖状态)
    static u8 key_debounce = 0;     // 去抖计数器
    static u8 key_last = 0;         // 上次检测到的按键值
    
    if(mode == 1) key_up = 1;       // 连按模式: 松开标志保持为1
    
    u8 key_now = 0;
    if(KEY0 == 1) key_now = KEY0_PRES;
    else if(WK_UP == 1) key_now = WKUP_PRES;
    
    if(key_now != 0)  // 有按键按下
    {
        if(key_now == key_last)  // 与上次相同
        {
            key_debounce++;
            if(key_debounce >= 2 && key_up)  // 连续2次检测到（约40ms去抖）
            {
                key_up = 0;          // 标记已触发一次
                key_debounce = 0;
                return key_now;      // 返回按键值
            }
        }
        else
        {
            key_debounce = 1;        // 按键变化，重新计数
            key_last = key_now;
        }
    }
    else  // 无按键按下
    {
        key_up = 1;                  // 允许下一次触发
        key_debounce = 0;
        key_last = 0;
    }
    
    return 0;   // 无按键按下或去抖中
}
