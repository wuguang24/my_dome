#include "WiFi_ESP8266.h"

#include <string.h>
#include <stdio.h>

#include "Delay.h"
#include "USART3.h"

// ESP8266 硬件连接约定：
// - 串口：USART2 (PA2=TX, PA3=RX)
// - 复位脚：PA1（WIFI_RESET_PIN）
#define WIFI_RESET_PORT GPIOA
#define WIFI_RESET_PIN  GPIO_Pin_1

#define WIFI_UART       USART2

// 接收缓冲区：在 USART2 RXNE 中断里逐字节写入。
// 说明：本驱动采用“简单累积字符串 + 关键字匹配”的方式判断 AT 应答。
static uint8_t wifi_rx_buf[WIFI_RX_BUF_SIZE];
static volatile uint16_t wifi_rx_len = 0;
static volatile uint16_t wifi_rx_len_prev = 0;

const uint8_t *WiFi_ESP8266_DebugGetRxBuf(void)
{
    return wifi_rx_buf;
}

static void wifi_clear_rx(void)
{
    // 清空接收缓冲区与长度计数
    memset((void*)wifi_rx_buf, 0, sizeof(wifi_rx_buf));
    wifi_rx_len = 0;
    wifi_rx_len_prev = 0;
}

static uint8_t wifi_wait_receive_stable(void)
{
    // “接收稳定”判定：
    // - 第一次发现 wifi_rx_len>0 先记录长度
    // - 下一次检测到长度没有继续增长，则认为一帧/一段应答基本接收完成
    // 该策略简单有效，但要求上层在合适的超时时间内轮询。
    if (wifi_rx_len == 0)
        return 0;

    if (wifi_rx_len == wifi_rx_len_prev)
    {
        wifi_rx_len_prev = 0;
        return 1;
    }

    wifi_rx_len_prev = wifi_rx_len;
    return 0;
}

static void wifi_send_raw(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        // 等待上一字节发送完成再写入（TC 标志）。
        // 这里偏“保守”，可保证串口发送顺序，但吞吐会略低。
        while (USART_GetFlagStatus(WIFI_UART, USART_FLAG_TC) == RESET);
        USART_SendData(WIFI_UART, data[i]);
    }
}

static void wifi_send_str(const char *s)
{
    wifi_send_raw((const uint8_t*)s, (uint16_t)strlen(s));
}

static uint8_t wifi_send_cmd_wait(const char *cmd, const char *expect, uint16_t timeout_10ms)
{
    char dbg_buf[192];

    // 发送前先清缓存，避免旧数据干扰关键字匹配
    wifi_clear_rx();
    snprintf(dbg_buf, sizeof(dbg_buf), "[WiFi] > %s", cmd);
    USART3_Send_Data((uint8_t*)dbg_buf, (uint8_t)strlen(dbg_buf));
    wifi_send_str(cmd);

    // timeout_10ms：以 10ms 为单位的超时计数
    while (timeout_10ms--)
    {
        if (wifi_wait_receive_stable())
        {
            // 关键字匹配：应答中包含 expect 则认为成功
            if (strstr((const char*)wifi_rx_buf, expect) != NULL)
            {
                wifi_clear_rx();
                return 1;
            }
        }
        Delay_ms(10);
    }

    snprintf(dbg_buf, sizeof(dbg_buf), "[WiFi] timeout rx=%s\r\n", (const char*)wifi_rx_buf);
    USART3_Send_Data((uint8_t*)dbg_buf, (uint8_t)strlen(dbg_buf));
    return 0;
}

void WiFi_ESP8266_RxIRQHandlerByte(uint8_t b)
{
    // USART2 接收中断回调：逐字节放入环形/线性缓冲。
    // 这里使用线性缓冲，满了就从头开始覆盖（避免越界）。
    if (wifi_rx_len >= WIFI_RX_BUF_SIZE)
        wifi_rx_len = 0;

    wifi_rx_buf[wifi_rx_len++] = b;
}

void WiFi_ESP8266_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // USART2_TX：PA2 复用推挽输出
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);

    // USART2_RX：PA3 浮空输入
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    usart.USART_BaudRate = baudrate;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(WIFI_UART, &usart);

    USART_ITConfig(WIFI_UART, USART_IT_RXNE, ENABLE);
    USART_Cmd(WIFI_UART, ENABLE);

    nvic.NVIC_IRQChannel = USART2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 2;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    // ESP8266 复位脚：推挽输出
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Pin = WIFI_RESET_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(WIFI_RESET_PORT, &gpio);

    // ESP8266 复位时序（经验值）：
    // - 先拉高保持一小段时间
    // - 再拉低一段时间触发复位
    // - 最后拉高等待模块启动并输出 ready/应答
    GPIO_SetBits(WIFI_RESET_PORT, WIFI_RESET_PIN);
    Delay_ms(10);
    GPIO_ResetBits(WIFI_RESET_PORT, WIFI_RESET_PIN);
    Delay_ms(250);
    GPIO_SetBits(WIFI_RESET_PORT, WIFI_RESET_PIN);
    Delay_ms(800);

    wifi_clear_rx();
    USART3_Send_Data((uint8_t*)"[WiFi] reset done, send AT\r\n", 27);
    wifi_send_cmd_wait("AT\r\n", "OK", 200);
    USART3_Send_Data((uint8_t*)"[WiFi] init done\r\n", 18);
}

uint8_t WiFi_ESP8266_ConnectAP(const char *ssid, const char *pwd)
{
    char cmd[160];

    if (!wifi_send_cmd_wait("AT\r\n", "OK", 200))
        return 0;

    if (!wifi_send_cmd_wait("AT+CWMODE=1\r\n", "OK", 200))
        return 0;

    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    if (!wifi_send_cmd_wait(cmd, "GOT IP", 2000))
    {
        const char *rx = (const char*)wifi_rx_buf;
        if (rx && (strstr(rx, "WIFI GOT IP") != NULL || strstr(rx, "GOT IP") != NULL))
        {
            wifi_clear_rx();
            return 1;
        }
        return 0;
    }

    return 1;
}

uint8_t WiFi_ESP8266_TCPConnect(const char *host, uint16_t port)
{
    char cmd[128];

    // 复位链路状态，避免因残留连接/透传模式导致 ERROR/CLOSED
    wifi_send_cmd_wait("AT+CIPCLOSE\r\n", "OK", 50);
    wifi_send_cmd_wait("AT+CIPMUX=0\r\n", "OK", 200);
    wifi_send_cmd_wait("AT+CIPMODE=0\r\n", "OK", 200);

    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u\r\n", host, port);
    return wifi_send_cmd_wait(cmd, "CONNECT", 800);
}

uint8_t WiFi_ESP8266_SendTCP(const uint8_t *data, uint16_t len)
{
    char cmd[40];

    wifi_clear_rx();
    // 进入发送长度设置：返回 '>' 表示可以开始透传发送数据
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u\r\n", len);
    if (!wifi_send_cmd_wait(cmd, ">", 200))
        return 0;

    wifi_send_raw(data, len);

    for (uint16_t t = 0; t < 120; t++)
    {
        if (wifi_wait_receive_stable())
        {
            // 常见发送结果关键字
            if (strstr((const char*)wifi_rx_buf, "SEND OK") != NULL)
            {
                wifi_clear_rx();
                return 1;
            }
            if (strstr((const char*)wifi_rx_buf, "SEND FAIL") != NULL ||
                strstr((const char*)wifi_rx_buf, "ERROR") != NULL ||
                strstr((const char*)wifi_rx_buf, "CLOSED") != NULL)
            {
                return 0;
            }
        }
        Delay_ms(10);
    }

    return 0;
}

uint8_t *WiFi_ESP8266_GetIPD(uint16_t timeout_10ms)
{
    char *ptr;

    // 从接收缓冲中查找 +IPD 数据：
    // 格式通常为：+IPD,<len>:<payload>
    // 本函数返回 payload 的起始指针（位于 ':' 之后）。
    while (timeout_10ms--)
    {
        if (wifi_wait_receive_stable())
        {
            ptr = strstr((char*)wifi_rx_buf, "IPD,");
            if (ptr)
            {
                ptr = strchr(ptr, ':');
                if (ptr)
                    return (uint8_t*)(ptr + 1);
                return 0;
            }
        }
        Delay_ms(10);
    }

    return 0;
}

void WiFi_ESP8266_Clear(void)
{
    wifi_clear_rx();
}
