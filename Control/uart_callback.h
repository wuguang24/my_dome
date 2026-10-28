#ifndef _UART_CALLBACK_H_
#define _UART_CALLBACK_H_
#include "board.h"

void BT_DAMConfig(void);

void BTBufferHandler(void);

extern volatile uint32_t BT_LastRxMs;
extern int Flag_Left, Flag_Right, Flag_Direction, Turn_Flag;

#endif 