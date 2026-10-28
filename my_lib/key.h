#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

// 按键引脚定义
#define KEY1_PIN    GPIO_Pin_12
#define KEY2_PIN    GPIO_Pin_11
#define KEY3_PIN    GPIO_Pin_10
#define KEY_PORT    GPIOA

#define KEY4_PIN    GPIO_Pin_9
#define KEY4_PORT   GPIOA

// 按键状态定义
typedef enum {
    KEY_NO_PRESS = 0,
    KEY_PRESS,
    KEY_LONG_PRESS
} KeyState;

// 按键ID定义
typedef enum {
    KEY_ID_1 = 0,
    KEY_ID_2,
    KEY_ID_3,
    KEY_ID_4,
    KEY_COUNT
} KeyID;

// 按键事件回调函数类型
typedef void (*KeyCallback)(void);

// 按键配置结构体
typedef struct {
    KeyID id;
    GPIO_TypeDef* port;
    uint16_t pin;
    KeyState state;
    uint32_t press_time;
    uint8_t is_pressed;
    KeyCallback press_callback;
    KeyCallback long_press_callback;
} KeyConfig;

// 按键初始化
void Key_Init(void);

// 按键扫描（非阻塞方式）
uint8_t Key_Scan_NonBlock(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

// 检查长按按键（用于退出设置模式）
uint8_t CheckLongPressKey1(void);
uint8_t CheckLongPressKey4(void);

// 设置按键回调函数
void Key_SetCallback(KeyID key_id, KeyCallback press_cb, KeyCallback long_press_cb);

// 按键处理任务（需要在主循环中调用）
void Key_ProcessTask(void);

// 获取按键按下状态
uint8_t Key_GetPressState(KeyID key_id);

// 清除按键状态
void Key_ClearState(KeyID key_id);

#endif /* __KEY_H */
