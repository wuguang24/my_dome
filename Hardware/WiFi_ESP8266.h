#ifndef __WIFI_ESP8266_H
#define __WIFI_ESP8266_H

#include "stm32f10x.h"

// ESP8266 AT 指令驱动接口（对应 WiFi_ESP8266.c）
// 说明：
// - 内部维护一个接收缓冲区，用于关键字匹配应答与提取 +IPD 数据
// - timeout_10ms 参数均表示“10ms 为单位”的超时计数
#define WIFI_RX_BUF_SIZE 512

void WiFi_ESP8266_Init(uint32_t baudrate);

// 连接 WiFi 热点（STA 模式）
uint8_t WiFi_ESP8266_ConnectAP(const char *ssid, const char *pwd);
// 连接 TCP 服务器
uint8_t WiFi_ESP8266_TCPConnect(const char *host, uint16_t port);

// 发送 TCP 数据（len 字节）
uint8_t WiFi_ESP8266_SendTCP(const uint8_t *data, uint16_t len);
// 从接收缓冲区提取 +IPD 负载指针（返回 payload 起始地址；找不到返回 0）
uint8_t *WiFi_ESP8266_GetIPD(uint16_t timeout_10ms);

// 清空接收缓冲区
void WiFi_ESP8266_Clear(void);

// 调试用途：获取当前接收缓冲区指针（只读）
const uint8_t *WiFi_ESP8266_DebugGetRxBuf(void);

// USART2 RXNE 中断中逐字节调用，用于填充接收缓冲区
void WiFi_ESP8266_RxIRQHandlerByte(uint8_t b);

#endif
