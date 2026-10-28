#include "stm32f10x.h"
#include "Delay.h"
#include "Key.h"

// 按键驱动
// 说明：GPIO 使用上拉输入（GPIO_Mode_IPU），因此：
// - 松开：读到 1
// - 按下：读到 0（低电平有效）

// 按键引脚定义
#define KEY_MODE_PIN  GPIO_Pin_0   // PB0
#define KEY_SEL_PIN  GPIO_Pin_1   // PB1
#ifdef ENABLE_UART_PRINT
#define KEY_UP_PIN  GPIO_Pin_15  // PB15
#else
#define KEY_UP_PIN  GPIO_Pin_10  // PB10
#endif
#define KEY_DOWN_PIN  GPIO_Pin_11  // PB11

// 按键初始化
void Key_Init(void)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY_MODE_PIN | KEY_SEL_PIN | KEY_UP_PIN | KEY_DOWN_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}


// 阻塞式按键读取（原功能保持）
// 特点：
// - 含 Delay_ms 消抖
// - 会一直等待按键松开（while...），因此会阻塞主循环
// - 适合简单演示，不适合对实时性要求高的任务
uint8_t Key_GetNum(void)
{
    uint8_t KeyNum = 0;
    
    if (GPIO_ReadInputDataBit(GPIOB, KEY_MODE_PIN) == 0)
    {
        Delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOB, KEY_MODE_PIN) == 0);
        Delay_ms(20);
        KeyNum = 1;
    }
    
    if (GPIO_ReadInputDataBit(GPIOB, KEY_SEL_PIN) == 0)
    {
        Delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOB, KEY_SEL_PIN) == 0);
        Delay_ms(20);
        KeyNum = 2;
    }

    if (GPIO_ReadInputDataBit(GPIOB, KEY_UP_PIN) == 0)
    {
        Delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOB, KEY_UP_PIN) == 0);
        Delay_ms(20);
        KeyNum = 3;
    }
		
		if (GPIO_ReadInputDataBit(GPIOB, KEY_DOWN_PIN) == 0)
    {
        Delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOB, KEY_DOWN_PIN) == 0);
        Delay_ms(20);
        KeyNum = 4;
    }
    
    return KeyNum;
}


// 非阻塞式按键扫描（新增，用于实时响应）
// 机制：下降沿检测（last_state=1 且 current_state=0）判定“按下一次”。
// 注意：该函数本身不做延时消抖；如果按键抖动明显，可能会产生误触发。
// 若需要更可靠，可在业务层增加时间窗口/计数消抖。
uint8_t Key_Scan(void)
{
    static uint8_t last_state[4] = {1, 1, 1, 1};  // 保存上次状态
    uint8_t current_state[4];
    uint8_t key_pressed = 0;
    
    // 读取当前按键状态
    current_state[0] = GPIO_ReadInputDataBit(GPIOB, KEY_MODE_PIN);
    current_state[1] = GPIO_ReadInputDataBit(GPIOB, KEY_SEL_PIN);
    current_state[2] = GPIO_ReadInputDataBit(GPIOB, KEY_UP_PIN);
    current_state[3] = GPIO_ReadInputDataBit(GPIOB, KEY_DOWN_PIN);
    
    // 检查按键1（下降沿检测）
    if(last_state[0] == 1 && current_state[0] == 0)
    {
        key_pressed = 1;//MODE
    }
    
    // 检查按键2
    else if(last_state[1] == 1 && current_state[1] == 0)
    {
        key_pressed = 2;//sel
    }

		// 检查按键3
		else if(last_state[2] == 1 && current_state[2] == 0)
    {
        key_pressed = 3;//up
    }

		// 检查按键4
		else if(last_state[3] == 1 && current_state[3] == 0)
    {
        key_pressed = 4;//down
    }
    
    // 更新上次状态
    last_state[0] = current_state[0];
    last_state[1] = current_state[1];
    last_state[2] = current_state[2];
    last_state[3] = current_state[3];
    
    return key_pressed;
}
