#ifndef __ONENET_MQTT_H
#define __ONENET_MQTT_H

#include "stm32f10x.h"

void OneNet_MQTT_Init(void);

uint8_t OneNet_MQTT_Connect(void);
void OneNet_MQTT_SubscribePropertySet(void);

void OneNet_MQTT_Tick100ms(void);

void OneNet_MQTT_ReportOnce(uint8_t temperature, uint8_t humidity, int light_lx, uint16_t co2_ppm,
                            uint8_t fan_on, uint8_t light_on, uint8_t humidifier_on);

void OneNet_MQTT_ProcessDownlink(uint8_t *ipd_payload);

#endif
