#ifndef __HCSR04_H
#define __HCSR04_H

#include "ti_msp_dl_config.h"

uint32_t Systick_getTick(void);
void delay_us(uint32_t us);

float HCSR04_ReadDistanceCm(void);

void HCSR04_AsyncStart(uint32_t now_ms);
int HCSR04_AsyncPoll(float *out_cm, uint32_t now_ms);
void HCSR04_AsyncOnEchoEdge(uint8_t level_high, uint32_t tick_val);
uint8_t HCSR04_AsyncIsBusy(void);

uint8_t HCSR04_AsyncGetState(void);
int HCSR04_AsyncGetFailCode(void);
uint8_t HCSR04_AsyncGetFailStreak(void);
void HCSR04_ForceReset(void);

#endif
