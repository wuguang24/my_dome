#include "stm32f10x.h"
#include "esp8266.h"
#include "Delay.h"
#include "usart3.h"
#include "aliyun_conf.h"
 #include "dbg.h"

#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO   "AT+CWJAP=\"" ALIYUN_WIFI_SSID "\",\"" ALIYUN_WIFI_PASS "\"\r\n"
#define ESP8266_MQTT_INFO   "AT+CIPSTART=\"TCP\",\"" ALIYUN_MQTT_HOST "\",1883\r\n"

unsigned char esp8266_buf[512];
static unsigned short esp8266_cnt = 0;
static unsigned short esp8266_cntPre = 0;

typedef enum
{
    ESP_ST_IDLE = 0,
    ESP_ST_AT,
    ESP_ST_ECHO_OFF,
    ESP_ST_RST,
    ESP_ST_MODE,
    ESP_ST_DHCP,
    ESP_ST_JOIN_AP,
    ESP_ST_TCP_START,
    ESP_ST_READY
} EspConnState;

static EspConnState s_state = ESP_ST_IDLE;
static uint32_t s_next_action_ms = 0;
static uint8_t s_tcp_connected = 0;
static EspConnState s_last_log_state = (EspConnState)0xFF;
static uint8_t s_at_fail_cnt = 0;
static uint8_t s_baud_idx = 0;

static const uint32_t s_baud_list[] = {115200, 9600, 57600, 38400};
static uint32_t ESP8266_GetBaud(void)
{
    return s_baud_list[s_baud_idx % (sizeof(s_baud_list) / sizeof(s_baud_list[0]))];
}

static void ESP8266_DumpRx(const char *tag)
{
    uint16_t i;
    uint16_t n = (esp8266_cnt < 80) ? esp8266_cnt : 80;

    DBG_PRINTF("[ESP][RXDUMP]%s len=%u: ", tag, (unsigned int)esp8266_cnt);
    for(i = 0; i < n; i++)
    {
        unsigned char c = esp8266_buf[i];
        if(c >= 32 && c <= 126)
            DBG_PRINTF("%c", c);
        else if(c == '\r')
            DBG_PRINTF("<r>");
        else if(c == '\n')
            DBG_PRINTF("<n>");
        else
            DBG_PRINTF("<%02X>", c);
    }
    DBG_PRINTF("\r\n");
}

static void ESP8266_LogState(EspConnState st)
{
    if(st == s_last_log_state) return;
    s_last_log_state = st;
    switch(st)
    {
        case ESP_ST_IDLE:      DBG_PRINTF("[ESP] IDLE\r\n"); break;
        case ESP_ST_AT:        DBG_PRINTF("[ESP] AT\r\n"); break;
        case ESP_ST_ECHO_OFF:  DBG_PRINTF("[ESP] WAIT_OK\r\n"); break;
        case ESP_ST_RST:       DBG_PRINTF("[ESP] RST\r\n"); break;
        case ESP_ST_MODE:      DBG_PRINTF("[ESP] CWMODE\r\n"); break;
        case ESP_ST_DHCP:      DBG_PRINTF("[ESP] DHCP\r\n"); break;
        case ESP_ST_JOIN_AP:   DBG_PRINTF("[ESP] JOIN_AP\r\n"); break;
        case ESP_ST_TCP_START: DBG_PRINTF("[ESP] TCP_START %s:%d\r\n", ALIYUN_MQTT_HOST, ALIYUN_MQTT_PORT); break;
        case ESP_ST_READY:     DBG_PRINTF("[ESP] READY tcp=1\r\n"); break;
        default:               DBG_PRINTF("[ESP] state=%d\r\n", (int)st); break;
    }
}

static void ESP8266_SendStr(const char *s)
{
    uart3_send((unsigned char *)s, 0xFF);
}

static uint8_t ESP8266_HasToken(const char *token)
{
    return (strstr((const char *)esp8266_buf, token) != NULL) ? 1U : 0U;
}

static void ESP8266_SendBytes(const unsigned char *data, unsigned short len)
{
    while(len--)
    {
        USART3_Sned_Char(*data++);
    }
}

uint8_t ESP8266_IsTcpConnected(void)
{
    return s_tcp_connected;
}

void ESP8266_Service(void)
{
    extern volatile uint32_t system_time_ms;
    uint32_t now = system_time_ms;

    if(s_state == ESP_ST_IDLE)
    {
        s_at_fail_cnt = 0;
        s_baud_idx = 0;
        USART3_Init(ESP8266_GetBaud());
        DBG_PRINTF("[ESP] USART3 baud=%lu\r\n", (unsigned long)ESP8266_GetBaud());
        ESP8266_Clear();
        s_tcp_connected = 0;
        s_next_action_ms = now;
        s_state = ESP_ST_AT;
        ESP8266_LogState(s_state);
    }

    // Passive disconnect detection
    if(ESP8266_HasToken("WIFI DISCONNECT"))
    {
        DBG_PRINTF("[ESP] WIFI DISCONNECT\r\n");
        s_tcp_connected = 0;
        ESP8266_Clear();
        s_state = ESP_ST_JOIN_AP;
        s_next_action_ms = now + 1000;
        ESP8266_LogState(s_state);
    }
    else if(ESP8266_HasToken("CLOSED"))
    {
        DBG_PRINTF("[ESP] TCP CLOSED\r\n");
        s_tcp_connected = 0;
        ESP8266_Clear();
        s_state = ESP_ST_TCP_START;
        s_next_action_ms = now + 500;
        ESP8266_LogState(s_state);
    }

    if((int32_t)(now - s_next_action_ms) < 0)
        return;

    switch(s_state)
    {
        case ESP_ST_AT:
            ESP8266_LogState(s_state);
            ESP8266_Clear();
            DBG_PRINTF("[ESP][TX] AT\\r\\n\r\n");
            ESP8266_SendStr("AT\r\n");
            s_next_action_ms = now + 800;
            s_state = ESP_ST_ECHO_OFF;
            break;

        case ESP_ST_ECHO_OFF:
            ESP8266_LogState(s_state);
            if(ESP8266_HasToken("OK"))
            {
                ESP8266_Clear();
                s_at_fail_cnt = 0;
                DBG_PRINTF("[ESP][TX] ATE0\\r\\n\r\n");
                ESP8266_SendStr("ATE0\r\n");
                s_next_action_ms = now + 800;
                s_state = ESP_ST_RST;
            }
            else
            {
                if(esp8266_cnt > 0)
                    ESP8266_DumpRx(" no_OK");

                // 如果一直收不到OK（甚至收不到任何回包），可能是ESP波特率不匹配：自动切换波特率尝试
                if(++s_at_fail_cnt >= 5)
                {
                    s_at_fail_cnt = 0;
                    s_baud_idx++;
                    USART3_Init(ESP8266_GetBaud());
                    DBG_PRINTF("[ESP] USART3 baud=%lu\r\n", (unsigned long)ESP8266_GetBaud());
                    ESP8266_Clear();
                }
                s_state = ESP_ST_AT;
                s_next_action_ms = now + 800;
            }
            break;

        case ESP_ST_RST:
            ESP8266_LogState(s_state);
            ESP8266_Clear();
            ESP8266_SendStr("AT+RST\r\n");
            s_next_action_ms = now + 2500;
            s_state = ESP_ST_MODE;
            break;

        case ESP_ST_MODE:
            ESP8266_LogState(s_state);
            ESP8266_Clear();
            ESP8266_SendStr("AT+CWMODE=1\r\n");
            s_next_action_ms = now + 800;
            s_state = ESP_ST_DHCP;
            break;

        case ESP_ST_DHCP:
            ESP8266_LogState(s_state);
            ESP8266_Clear();
            ESP8266_SendStr("AT+CWDHCP=1,1\r\n");
            s_next_action_ms = now + 800;
            s_state = ESP_ST_JOIN_AP;
            break;

        case ESP_ST_JOIN_AP:
            ESP8266_LogState(s_state);
            if(ESP8266_HasToken("GOT IP"))
            {
                DBG_PRINTF("[ESP] GOT IP\r\n");
                ESP8266_Clear();
                s_state = ESP_ST_TCP_START;
                s_next_action_ms = now + 200;
            }
            else
            {
                ESP8266_Clear();
                ESP8266_SendStr(ESP8266_WIFI_INFO);
                s_next_action_ms = now + 3000;
            }
            break;

        case ESP_ST_TCP_START:
            ESP8266_LogState(s_state);
            if(ESP8266_HasToken("CONNECT") || ESP8266_HasToken("ALREADY CONNECTED"))
            {
                ESP8266_Clear();
                s_tcp_connected = 1;
                s_state = ESP_ST_READY;
                s_next_action_ms = now + 1000;
                ESP8266_LogState(s_state);
            }
            else
            {
                ESP8266_Clear();
                ESP8266_SendStr(ESP8266_MQTT_INFO);
                s_next_action_ms = now + 2000;
            }
            break;

        case ESP_ST_READY:
            ESP8266_LogState(s_state);
            // Keep ready; disconnects handled by passive detection
            s_next_action_ms = now + 1000;
            break;

        default:
            s_state = ESP_ST_IDLE;
            s_next_action_ms = now + 1000;
            break;
    }
}

void ESP8266_RxHandler(unsigned char ch)
{
    if(esp8266_cnt >= sizeof(esp8266_buf)) esp8266_cnt = 0;
    esp8266_buf[esp8266_cnt++] = ch;
}

void ESP8266_Clear(void)
{
    memset(esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;
}

static _Bool ESP8266_WaitRecive(void)
{
    if(esp8266_cnt == 0) return REV_WAIT;

    if(esp8266_cnt == esp8266_cntPre)
    {
        esp8266_cnt = 0;
        return REV_OK;
    }

    esp8266_cntPre = esp8266_cnt;
    return REV_WAIT;
}

static _Bool ESP8266_SendCmd(char *cmd, char *res)
{
    unsigned short timeOut = 200;

    uart3_send((unsigned char *)cmd, 0xFF);

    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)
        {
            if(strstr((const char *)esp8266_buf, res) != NULL)
            {
                ESP8266_Clear();
                return 0;
            }
        }

        Delay_ms(10);
    }

    return 1;
}

void ESP8266_SendData(unsigned char *data, unsigned short len)
{
    char cmdBuf[32];

    ESP8266_Clear();
    sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);
    if(!ESP8266_SendCmd(cmdBuf, ">"))
    {
        ESP8266_SendBytes(data, len);
    }
}

unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{
    char *ptrIPD = NULL;

    if(timeOut == 0)
    {
        if(ESP8266_WaitRecive() != REV_OK)
            return NULL;

        ptrIPD = strstr((char *)esp8266_buf, "IPD,");
        if(ptrIPD == NULL)
            return NULL;

        ptrIPD = strchr(ptrIPD, ':');
        if(ptrIPD == NULL)
            return NULL;

        ptrIPD++;
        return (unsigned char *)(ptrIPD);
    }

    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)
        {
            ptrIPD = strstr((char *)esp8266_buf, "IPD,");
            if(ptrIPD != NULL)
            {
                ptrIPD = strchr(ptrIPD, ':');
                if(ptrIPD != NULL)
                {
                    ptrIPD++;
                    return (unsigned char *)(ptrIPD);
                }
                return NULL;
            }
        }

        Delay_ms(5);
    }

    return NULL;
}

void ESP8266_Init(void)
{
    USART3_Init(115200);

    ESP8266_Clear();

    while(ESP8266_SendCmd("AT\r\n", "OK")) Delay_ms(500);

    ESP8266_SendCmd("ATE0\r\n", "OK");

    while(ESP8266_SendCmd("AT+RST\r\n", "OK")) Delay_ms(500);
    Delay_ms(2000);

    while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK")) Delay_ms(500);

    while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK")) Delay_ms(500);

    while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP")) Delay_ms(500);

    while(ESP8266_SendCmd(ESP8266_MQTT_INFO, "CONNECT")) Delay_ms(500);
}
