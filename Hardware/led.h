#ifndef _LED_H
#define _LED_H
#include "../ti_msp_dl_config.h"


typedef enum {
	LED_MODE_NORMAL = 0,
	LED_MODE_AVOID,
	LED_MODE_ODOM_OFF
} LedMode_t;


void LED_ON(void);
void LED_OFF(void);
void LED_Toggle(void);
void LED_Flash(uint16_t time);

void LED_SetMode(LedMode_t mode);
void LED_PulseOn(uint16_t duration_ticks);
void LED_Task(void);
#endif 
