#ifndef __KEY_H
#define __KEY_H

// 按键模块接口（对应 Key.c）
// 返回值约定：
// - 0：无按键
// - 1：MODE
// - 2：SEL
// - 3：UP
// - 4：DOWN

void Key_Init(void);
// 阻塞式读取：带延时消抖且等待松手，可能阻塞主循环
uint8_t Key_GetNum(void);
// 非阻塞扫描：下降沿检测，适合主循环高频调用
uint8_t Key_Scan(void);  // 新增：非阻塞式按键扫描

#endif
