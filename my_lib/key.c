#include "key.h"
#include "stm32f10x.h"
#include <stdio.h>

// 按键配置数组
static KeyConfig key_configs[KEY_COUNT] = {
    {KEY_ID_1, KEY_PORT, KEY1_PIN, KEY_NO_PRESS, 0, 0, NULL, NULL},
    {KEY_ID_2, KEY_PORT, KEY2_PIN, KEY_NO_PRESS, 0, 0, NULL, NULL},
    {KEY_ID_3, KEY_PORT, KEY3_PIN, KEY_NO_PRESS, 0, 0, NULL, NULL},
    {KEY_ID_4, KEY4_PORT, KEY4_PIN, KEY_NO_PRESS, 0, 0, NULL, NULL}
};

// 全局时间变量（需要从main.c中获取）
extern volatile uint32_t system_time_ms;

// 获取系统时间（毫秒）
static uint32_t GetSystemTime(void)
{
    return system_time_ms;
}

// 按键初始化
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置按键引脚为上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN | KEY4_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

// 按键扫描（非阻塞方式）
uint8_t Key_Scan_NonBlock(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    static uint8_t key_state[KEY_COUNT] = {0};       // 按键状态机状态
    static uint32_t key_time[KEY_COUNT] = {0};       // 按键时间记录
    static uint8_t key_pressed[KEY_COUNT] = {0};     // 按键已按下标志
    
    uint8_t key_index = 0;
    
    // 确定按键索引
    if(GPIO_Pin == KEY1_PIN) key_index = 0;
    else if(GPIO_Pin == KEY2_PIN) key_index = 1;
    else if(GPIO_Pin == KEY3_PIN) key_index = 2;
    else if(GPIO_Pin == KEY4_PIN) key_index = 3;
    else return 0;
    
    // 检测按键当前状态
    if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == 0)
    {
        if(key_state[key_index] == 0)
        {
            // 按键刚刚按下
            key_state[key_index] = 1;
            key_time[key_index] = GetSystemTime();
        }
        else if(key_state[key_index] == 1)
        {
            // 检查是否已经过了消抖时间
            if((GetSystemTime() - key_time[key_index]) > 20)
            {
                if(!key_pressed[key_index])
                {
                    key_pressed[key_index] = 1;
                    key_state[key_index] = 2;
                    return 1;  // 按键按下
                }
            }
        }
    }
    else
    {
        if(key_state[key_index] != 0)
        {
            // 按键释放
            key_state[key_index] = 0;
            key_pressed[key_index] = 0;
        }
    }
    
    return 0;
}

// 检查长按按键（用于退出设置模式）
uint8_t CheckLongPressKey1(void)
{
    static uint8_t long_press_detected = 0;
    static uint32_t press_start_time = 0;
    
    if(GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN) == 0)
    {
        if(press_start_time == 0)
        {
            press_start_time = GetSystemTime();
        }
        else if((GetSystemTime() - press_start_time) > 1000)
        {
            if(!long_press_detected)
            {
                long_press_detected = 1;
                return 1;
            }
        }
    }
    else
    {
        press_start_time = 0;
        long_press_detected = 0;
    }
    
    return 0;
}

uint8_t CheckLongPressKey4(void)
{
    static uint8_t long_press_detected = 0;
    static uint32_t press_start_time = 0;
 
    if(GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN) == 0)
    {
        if(press_start_time == 0)
        {
            press_start_time = GetSystemTime();
        }
        else if((GetSystemTime() - press_start_time) > 1000)
        {
            if(!long_press_detected)
            {
                long_press_detected = 1;
                return 1;
            }
        }
    }
    else
    {
        press_start_time = 0;
        long_press_detected = 0;
    }
 
    return 0;
}

// 设置按键回调函数
void Key_SetCallback(KeyID key_id, KeyCallback press_cb, KeyCallback long_press_cb)
{
    if(key_id < KEY_COUNT)
    {
        key_configs[key_id].press_callback = press_cb;
        key_configs[key_id].long_press_callback = long_press_cb;
    }
}

// 按键处理任务（需要在主循环中调用）
void Key_ProcessTask(void)
{
    for(uint8_t i = 0; i < KEY_COUNT; i++)
    {
        // 检查按键是否按下
        if(Key_Scan_NonBlock(key_configs[i].port, key_configs[i].pin))
        {
            key_configs[i].state = KEY_PRESS;
            key_configs[i].press_time = GetSystemTime();
            key_configs[i].is_pressed = 1;
            
            // 执行短按回调
            if(key_configs[i].press_callback != NULL)
            {
                key_configs[i].press_callback();
            }
        }
        
        // 检查长按
        if(key_configs[i].is_pressed)
        {
            if(GPIO_ReadInputDataBit(key_configs[i].port, key_configs[i].pin) == 0)
            {
                if((GetSystemTime() - key_configs[i].press_time) > 1000)
                {
                    if(key_configs[i].state != KEY_LONG_PRESS)
                    {
                        key_configs[i].state = KEY_LONG_PRESS;
                        
                        // 执行长按回调
                        if(key_configs[i].long_press_callback != NULL)
                        {
                            key_configs[i].long_press_callback();
                        }
                    }
                }
            }
            else
            {
                key_configs[i].is_pressed = 0;
                if(key_configs[i].state == KEY_LONG_PRESS)
                {
                    key_configs[i].state = KEY_NO_PRESS;
                }
            }
        }
    }
}

// 获取按键按下状态
uint8_t Key_GetPressState(KeyID key_id)
{
    if(key_id < KEY_COUNT)
    {
        return (key_configs[key_id].state == KEY_PRESS);
    }
    return 0;
}

// 清除按键状态
void Key_ClearState(KeyID key_id)
{
    if(key_id < KEY_COUNT)
    {
        key_configs[key_id].state = KEY_NO_PRESS;
        key_configs[key_id].is_pressed = 0;
        key_configs[key_id].press_time = 0;
    }
}
