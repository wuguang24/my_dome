#ifndef _ONENET_H_
#define _ONENET_H_

#include "stm32f10x.h"

_Bool OneNet_DevLink(void);
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt);
void OneNet_Publish(const char *topic, const char *msg);
void OneNet_RevPro(unsigned char *cmd);

const char *OneNet_GetRxPayload(void);
void OneNet_ClearRxPayload(void);

void OneNet_ConfigSubscribeTopic(const char *topic_set);
void OneNet_Service(void);
uint8_t OneNet_IsMqttConnected(void);
uint8_t OneNet_IsSubscribed(void);

#endif
