#ifndef _ESP8266_H_
#define _ESP8266_H_

#define REV_OK      0
#define REV_WAIT    1

extern unsigned char esp8266_buf[512];

void ESP8266_Init(void);
void ESP8266_Clear(void);
void ESP8266_SendData(unsigned char *data, unsigned short len);
unsigned char *ESP8266_GetIPD(unsigned short timeOut);

void ESP8266_RxHandler(unsigned char ch);

void ESP8266_Service(void);
uint8_t ESP8266_IsTcpConnected(void);

#endif
