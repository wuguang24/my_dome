#ifndef __GPIO_H
#define __GPIO_H	 
#include "stm32f10x.h" 

#define KEY1 PAin(3)
#define KEY2 PAin(4)
#define KEY3 PAin(5)

#define BEEP   PCout(15) 
#define RELAY  PBout(9) //继电器
#define MOTOR  PBout(8) //电机/水泵
#define LED    PAout(7) 

void KEY_GPIO_Init(void);//引脚初始化
void MOTOR_GPIO_Init(void);

#endif
