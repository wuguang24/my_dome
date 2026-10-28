#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"                  // Device header
#include "oled.h"

extern uint8_t Usart3_RxPacket[10];				//����������ݰ�����
extern uint8_t Usart3_RxFlag;
extern uint8_t Usart3_seshig;

void Uart3_Init(void);
uint8_t CalXorSum(const uint8_t *data, uint32_t len);
void Usart3_SendBytes(const uint8_t *data, uint32_t len);
void Usart3_SendPacket(uint8_t func, uint8_t cmd);

#endif


