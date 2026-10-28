#include "OneNet_MQTT.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "WiFi_ESP8266.h"
#include "MqttKit.h"

#include "Actuator.h"
#include "USART3.h"

extern int16_t low_limit[4];
extern int16_t high_limit[4];

extern volatile uint8_t remote_override;
extern volatile uint8_t remote_fan_on;
extern volatile uint8_t remote_light_on;
extern volatile uint8_t remote_hum_on;
extern volatile uint8_t remote_override_tmo_100ms;

#define ONENET_HOST "mqtts.heclouds.com"
#define ONENET_PORT 1883

#ifndef WIFI_SSID
#define WIFI_SSID "baek"
#endif

#ifndef WIFI_PWD
#define WIFI_PWD "15915904"
#endif

#ifndef ONENET_PRODUCT_ID
#define ONENET_PRODUCT_ID "K7BgAk6bPS"
#endif

#ifndef ONENET_DEVICE_NAME
#define ONENET_DEVICE_NAME "test"
#endif

#ifndef ONENET_AUTH_INFO
#define ONENET_AUTH_INFO "version=2018-10-31&res=products%2FK7BgAk6bPS%2Fdevices%2Ftest&et=1789732931&method=md5&sign=yfVrCSWTFC1QJIw7miqCdA%3D%3D"
#endif

static uint8_t g_mqtt_connected = 0;
static uint8_t g_tcp_connected = 0;
static uint16_t g_report_tick = 0;

typedef enum
{
    ONENET_STATE_IDLE = 0,
    ONENET_STATE_WIFI_JOIN,
    ONENET_STATE_TCP_CONNECT,
    ONENET_STATE_MQTT_CONNECT,
    ONENET_STATE_SUBSCRIBE,
    ONENET_STATE_READY
} OneNetState;

static OneNetState g_state = ONENET_STATE_IDLE;
static uint8_t g_retry_div = 0;
static uint8_t g_tcp_backoff_100ms = 0;

static void dbg_print(const char *s)
{
    USART3_Send_Data((uint8_t*)s, (uint8_t)strlen(s));
}

static void dbg_printf(const char *fmt, ...)
{
    char buf[192];
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n <= 0) return;
    if (n >= (int)sizeof(buf)) n = (int)sizeof(buf) - 1;
    USART3_Send_Data((uint8_t*)buf, (uint8_t)n);
}

static uint32_t parse_auth_et(const char *auth)
{
    const char *p;
    if (!auth) return 0;
    p = strstr(auth, "et=");
    if (!p) return 0;
    p += 3;
    return (uint32_t)strtoul(p, NULL, 10);
}

static void dbg_print_masked(const char *tag, const uint8_t *s, uint16_t len)
{
    char buf[128];
    uint16_t head = len;
    if (head > 16) head = 16;

    if (len == 0 || !s)
    {
        dbg_printf("[OneNet] %s len=0\r\n", tag);
        return;
    }

    memcpy(buf, s, head);
    buf[head] = 0;
    dbg_printf("[OneNet] %s len=%u head=\"%s\"\r\n", tag, (unsigned)len, buf);
}

static void dbg_dump_connect_packet(const MQTT_PACKET_STRUCTURE *pkt)
{
    uint32_t remain_len = 0;
    uint8_t len_bytes = 0;
    const uint8_t *p;
    const uint8_t *end;
    uint16_t slen;
    uint8_t flags;
    uint16_t keepalive;

    if (!pkt || !pkt->_data || pkt->_len < 10) return;
    if ((pkt->_data[0] >> 4) != MQTT_PKT_CONNECT) return;

    // Remaining length decode
    p = pkt->_data + 1;
    {
        uint32_t mul = 1;
        uint8_t digit;
        do
        {
            if (len_bytes >= 4) return;
            digit = *p++;
            remain_len += (digit & 0x7F) * mul;
            mul *= 128;
            len_bytes++;
        } while (digit & 0x80);
    }

    end = pkt->_data + 1 + len_bytes + remain_len;
    if (end > pkt->_data + pkt->_len) end = pkt->_data + pkt->_len;

    // Protocol name
    if (p + 2 > end) return;
    slen = (uint16_t)((p[0] << 8) | p[1]);
    p += 2;
    if (p + slen + 4 > end) return;
    p += slen;           // skip "MQTT"
    p += 1;              // protocol level
    flags = *p++;        // connect flags
    keepalive = (uint16_t)((p[0] << 8) | p[1]);
    p += 2;

    dbg_printf("[OneNet] CONNECT flags=0x%02X keepalive=%u\r\n", (unsigned)flags, (unsigned)keepalive);

    // ClientId
    if (p + 2 > end) return;
    slen = (uint16_t)((p[0] << 8) | p[1]);
    p += 2;
    if (p + slen > end) return;
    dbg_print_masked("ClientId", p, slen);
    p += slen;

    // Username
    if (flags & MQTT_CONNECT_USER_NAME)
    {
        if (p + 2 > end) return;
        slen = (uint16_t)((p[0] << 8) | p[1]);
        p += 2;
        if (p + slen > end) return;
        dbg_print_masked("Username", p, slen);
        p += slen;
    }

    // Password
    if (flags & MQTT_CONNECT_PASSORD)
    {
        if (p + 2 > end) return;
        slen = (uint16_t)((p[0] << 8) | p[1]);
        p += 2;
        if (p + slen > end) return;
        // password/token 只打印长度和前16字节，避免泄露
        dbg_print_masked("Password", p, slen);
        p += slen;
    }
}

void OneNet_MQTT_Init(void)
{
    g_mqtt_connected = 0;
    g_tcp_connected = 0;
    g_report_tick = 0;
    g_state = ONENET_STATE_WIFI_JOIN;
    g_retry_div = 0;
    g_tcp_backoff_100ms = 0;
}

uint8_t OneNet_MQTT_Connect(void)
{
    MQTT_PACKET_STRUCTURE pkt = {0};
    uint8_t *ipd = 0;

    dbg_printf("[OneNet] proid=%s dev=%s auth_len=%u\r\n",
               ONENET_PRODUCT_ID, ONENET_DEVICE_NAME, (unsigned)strlen(ONENET_AUTH_INFO));

    dbg_printf("[OneNet] auth_et=%lu\r\n", (unsigned long)parse_auth_et(ONENET_AUTH_INFO));

    if (!g_tcp_connected)
    {
        if (!WiFi_ESP8266_TCPConnect(ONENET_HOST, ONENET_PORT))
        {
            dbg_print("[OneNet] TCP connect fail\r\n");
            return 0;
        }
        g_tcp_connected = 1;
    }

    if (MQTT_PacketConnect(ONENET_PRODUCT_ID, ONENET_AUTH_INFO, ONENET_DEVICE_NAME,
                           256, 1, MQTT_QOS_LEVEL0, 0, 0, 0, &pkt) != 0)
    {
        dbg_print("[OneNet] MQTT connect packet fail\r\n");
        return 0;
    }

    dbg_printf("[OneNet] CONNECT len=%u\r\n", (unsigned)pkt._len);
    dbg_dump_connect_packet(&pkt);
    if (!WiFi_ESP8266_SendTCP(pkt._data, (uint16_t)pkt._len))
    {
        dbg_printf("[WiFi] CIPSEND fail(CONNECT) rx=%s\r\n", (const char*)WiFi_ESP8266_DebugGetRxBuf());
        MQTT_DeleteBuffer(&pkt);
        g_tcp_connected = 0;
        return 0;
    }
    MQTT_DeleteBuffer(&pkt);

    ipd = WiFi_ESP8266_GetIPD(250);
    if (!ipd)
    {
        dbg_print("[OneNet] no CONNACK\r\n");
        return 0;
    }

    if (MQTT_UnPacketRecv(ipd) == MQTT_PKT_CONNACK)
    {
        uint8_t ack = MQTT_UnPacketConnectAck(ipd);
        if (ack == 0)
        {
            g_mqtt_connected = 1;
            dbg_print("[OneNet] MQTT connected\r\n");
            return 1;
        }
        dbg_printf("[OneNet] CONNACK code=%u\r\n", (unsigned)ack);
        if (ack == 4)
            dbg_print("[OneNet] WARN: bad username/password (check AUTH_INFO/token)\r\n");
    }

    dbg_print("[OneNet] CONNACK fail\r\n");
    return 0;
}

void OneNet_MQTT_SubscribePropertySet(void)
{
    MQTT_PACKET_STRUCTURE pkt = {0};
    char topic_buf[128];
    const char *topic = topic_buf;

    if (!g_mqtt_connected) return;

    snprintf(topic_buf, sizeof(topic_buf), "$sys/%s/%s/thing/property/set", ONENET_PRODUCT_ID, ONENET_DEVICE_NAME);

    dbg_printf("[OneNet] SUB topic=%s\r\n", topic_buf);

    if (MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, (const int8**)&topic, 1, &pkt) == 0)
    {
        dbg_printf("[OneNet] SUB len=%u\r\n", (unsigned)pkt._len);
        if (!WiFi_ESP8266_SendTCP(pkt._data, (uint16_t)pkt._len))
        {
            dbg_print("[WiFi] CIPSEND fail(SUB)\r\n");
            MQTT_DeleteBuffer(&pkt);
            g_tcp_connected = 0;
            g_mqtt_connected = 0;
            return;
        }
        MQTT_DeleteBuffer(&pkt);
        dbg_print("[OneNet] subscribed\r\n");
    }
}

static uint16_t build_property_post_json(char *out, uint16_t out_size,
                                        uint8_t temperature, uint8_t humidity, int light_lx, uint16_t co2_ppm,
                                        uint8_t fan_on, uint8_t light_on, uint8_t humidifier_on)
{
    int n = snprintf(out, out_size,
        "{\"id\":\"1\",\"version\":\"1.0\",\"params\":{" 
        "\"temp\":{\"value\":%u},"
        "\"humi\":{\"value\":%u},"
        "\"light\":{\"value\":%d},"
        "\"co2\":{\"value\":%u},"
        "\"fan\":{\"value\":%s},"
        "\"light_sw\":{\"value\":%s},"
        "\"humidifier\":{\"value\":%s}"
        "}}}",
        (unsigned)temperature, (unsigned)humidity, light_lx, (unsigned)co2_ppm,
        fan_on ? "true" : "false",
        light_on ? "true" : "false",
        humidifier_on ? "true" : "false");

    if (n <= 0) return 0;
    if (n >= out_size) return (uint16_t)(out_size - 1);
    return (uint16_t)n;
}

void OneNet_MQTT_ReportOnce(uint8_t temperature, uint8_t humidity, int light_lx, uint16_t co2_ppm,
                            uint8_t fan_on, uint8_t light_on, uint8_t humidifier_on)
{
    MQTT_PACKET_STRUCTURE pkt = {0};
    char json[256];
    char topic[128];
    uint16_t json_len;

    if (!g_mqtt_connected) return;

    json_len = build_property_post_json(json, sizeof(json), temperature, humidity, light_lx, co2_ppm,
                                        fan_on, light_on, humidifier_on);
    if (json_len == 0) return;

    snprintf(topic, sizeof(topic), "$sys/%s/%s/thing/property/post", ONENET_PRODUCT_ID, ONENET_DEVICE_NAME);

    dbg_printf("[OneNet] POST json=%u\r\n", (unsigned)json_len);

    if (MQTT_PacketPublish(MQTT_PUBLISH_ID, (const int8*)topic,
                           (const int8*)json, json_len, MQTT_QOS_LEVEL0, 0, 0, &pkt) == 0)
    {
        dbg_printf("[OneNet] PUB len=%u\r\n", (unsigned)pkt._len);
        if (!WiFi_ESP8266_SendTCP(pkt._data, (uint16_t)pkt._len))
        {
            dbg_print("[WiFi] CIPSEND fail(PUB)\r\n");
            MQTT_DeleteBuffer(&pkt);
            g_tcp_connected = 0;
            g_mqtt_connected = 0;
            return;
        }
        MQTT_DeleteBuffer(&pkt);
    }
}

static void apply_param_int(const char *name, int v)
{
    if (strcmp(name, "temp_high") == 0)
        high_limit[0] = (int16_t)v;
    else if (strcmp(name, "temp_low") == 0)
        low_limit[0] = (int16_t)v;
    else if (strcmp(name, "humi_low") == 0)
        low_limit[1] = (int16_t)v;
    else if (strcmp(name, "humi_high") == 0)
        high_limit[1] = (int16_t)v;
    else if (strcmp(name, "light_low") == 0)
        low_limit[2] = (int16_t)v;
    else if (strcmp(name, "light_high") == 0)
        high_limit[2] = (int16_t)v;
    else if (strcmp(name, "co2_high") == 0)
        high_limit[3] = (int16_t)v;
    else if (strcmp(name, "co2_low") == 0)
        low_limit[3] = (int16_t)v;
}

static int json_find_value_int(const char *json, const char *key, int *out)
{
    char pat[64];
    char *p;
    char *v;

    if (!json || !key || !out) return 0;

    snprintf(pat, sizeof(pat), "\"%s\"", key);
    p = strstr((char*)json, pat);
    if (!p) return 0;

    v = strstr(p, "\"value\"");
    if (!v) return 0;
    v = strchr(v, ':');
    if (!v) return 0;
    v++;
    while (*v == ' ' || *v == '\t') v++;

    if (strncmp(v, "true", 4) == 0)
    {
        *out = 1;
        return 1;
    }
    if (strncmp(v, "false", 5) == 0)
    {
        *out = 0;
        return 1;
    }

    *out = atoi(v);
    return 1;
}

static int json_find_value_int_simple(const char *json, const char *key, int *out)
{
    char pat[64];
    char *p;
    char *v;

    if (!json || !key || !out) return 0;

    snprintf(pat, sizeof(pat), "\"%s\"", key);
    p = strstr((char*)json, pat);
    if (!p) return 0;

    v = strchr(p, ':');
    if (!v) return 0;
    v++;
    while (*v == ' ' || *v == '\t') v++;

    if (strncmp(v, "true", 4) == 0)
    {
        *out = 1;
        return 1;
    }
    if (strncmp(v, "false", 5) == 0)
    {
        *out = 0;
        return 1;
    }

    *out = atoi(v);
    return 1;
}

static int json_find_value_int_any(const char *json, const char *key, int *out)
{
    if (json_find_value_int(json, key, out)) return 1;
    return json_find_value_int_simple(json, key, out);
}

static int json_find_id_str(const char *json, char *out, uint16_t out_size)
{
    char *p;
    char *q;
    uint16_t n;

    if (!json || !out || out_size == 0) return 0;

    p = strstr((char*)json, "\"id\"");
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\"')
    {
        p++;
        q = strchr(p, '\"');
        if (!q) return 0;
        n = (uint16_t)(q - p);
    }
    else
    {
        q = p;
        while (*q >= '0' && *q <= '9') q++;
        if (q == p) return 0;
        n = (uint16_t)(q - p);
    }
    if (n >= out_size) n = (uint16_t)(out_size - 1);
    memcpy(out, p, n);
    out[n] = 0;
    return 1;
}

void OneNet_MQTT_ProcessDownlink(uint8_t *ipd_payload)
{
    int8 *topic = 0;
    int8 *payload = 0;
    uint16 topic_len = 0;
    uint16 payload_len = 0;
    uint8 qos = 0;
    uint16 pkt_id = 0;

    if (!ipd_payload) return;

    if (MQTT_UnPacketRecv(ipd_payload) != MQTT_PKT_PUBLISH) return;

    if (MQTT_UnPacketPublish(ipd_payload, &topic, &topic_len, &payload, &payload_len, &qos, &pkt_id) != 0)
        return;

    if (payload_len == 0) return;

    if (topic_len == 0 || !topic) return;
    {
        char topic_str[160];
        uint16_t n = topic_len;
        if (n >= sizeof(topic_str)) n = (uint16_t)(sizeof(topic_str) - 1);
        memcpy(topic_str, topic, n);
        topic_str[n] = 0;
        dbg_printf("[OneNet] topic=%s\r\n", topic_str);
        if (strstr(topic_str, "thing/property/set") == NULL)
            return;
    }

    {
        char tmp[400];
        char id[24];
        uint16_t copy_len = payload_len;
        int v;

        uint8_t got_sw = 0;

        if (copy_len >= sizeof(tmp)) copy_len = sizeof(tmp) - 1;
        memcpy(tmp, payload, copy_len);
        tmp[copy_len] = 0;

        dbg_printf("[OneNet] downlink=%s\r\n", tmp);

        if (json_find_value_int_any(tmp, "fan", &v))
        {
            remote_fan_on = (uint8_t)v;
            got_sw = 1;
            dbg_printf("[OneNet] set fan=%d\r\n", v);
        }
        if (json_find_value_int_any(tmp, "light_sw", &v))
        {
            remote_light_on = (uint8_t)v;
            got_sw = 1;
            dbg_printf("[OneNet] set light_sw=%d\r\n", v);
        }
        if (json_find_value_int_any(tmp, "light", &v))
        {
            remote_light_on = (uint8_t)v;
            got_sw = 1;
            dbg_printf("[OneNet] set light=%d\r\n", v);
        }
        if (json_find_value_int_any(tmp, "humidifier", &v))
        {
            remote_hum_on = (uint8_t)v;
            got_sw = 1;
            dbg_printf("[OneNet] set humidifier=%d\r\n", v);
        }
        if (json_find_value_int_any(tmp, "humidifier_sw", &v))
        {
            remote_hum_on = (uint8_t)v;
            got_sw = 1;
            dbg_printf("[OneNet] set humidifier_sw=%d\r\n", v);
        }
        if (json_find_value_int_any(tmp, "hum", &v))
        {
            remote_hum_on = (uint8_t)v;
            got_sw = 1;
            dbg_printf("[OneNet] set hum=%d\r\n", v);
        }

        if (got_sw)
        {
            if (remote_fan_on == 0 && remote_light_on == 0 && remote_hum_on == 0)
            {
                remote_override = 0;
                remote_override_tmo_100ms = 0;
            }
            else
            {
                remote_override = 1;
                remote_override_tmo_100ms = 30;
            }
            dbg_printf("[OneNet] remote_override=%u fan=%u light=%u hum=%u\r\n",
                       (unsigned)remote_override,
                       (unsigned)remote_fan_on, (unsigned)remote_light_on, (unsigned)remote_hum_on);
        }

        if (json_find_value_int_any(tmp, "temp_high", &v))
            apply_param_int("temp_high", v);
        if (json_find_value_int_any(tmp, "temp_low", &v))
            apply_param_int("temp_low", v);
        if (json_find_value_int_any(tmp, "humi_low", &v))
            apply_param_int("humi_low", v);
        if (json_find_value_int_any(tmp, "humi_high", &v))
            apply_param_int("humi_high", v);
        if (json_find_value_int_any(tmp, "light_low", &v))
            apply_param_int("light_low", v);
        if (json_find_value_int_any(tmp, "light_high", &v))
            apply_param_int("light_high", v);
        if (json_find_value_int_any(tmp, "co2_high", &v))
            apply_param_int("co2_high", v);
        if (json_find_value_int_any(tmp, "co2_low", &v))
            apply_param_int("co2_low", v);

        if (json_find_id_str(tmp, id, sizeof(id)))
        {
            MQTT_PACKET_STRUCTURE pkt = {0};
            char reply_topic[128];
            char reply_json[96];
            uint16_t reply_len;

            snprintf(reply_topic, sizeof(reply_topic), "$sys/%s/%s/thing/property/set_reply", ONENET_PRODUCT_ID, ONENET_DEVICE_NAME);
            snprintf(reply_json, sizeof(reply_json), "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}", id);
            reply_len = (uint16_t)strlen(reply_json);

            if (MQTT_PacketPublish(MQTT_PUBLISH_ID, (const int8*)reply_topic,
                                   (const int8*)reply_json, reply_len, MQTT_QOS_LEVEL0, 0, 0, &pkt) == 0)
            {
                if (WiFi_ESP8266_SendTCP(pkt._data, (uint16_t)pkt._len))
                {
                    dbg_print("[OneNet] set_reply ok\r\n");
                }
                else
                {
                    dbg_print("[OneNet] set_reply fail\r\n");
                }
                MQTT_DeleteBuffer(&pkt);
            }
        }
    }
}

void OneNet_MQTT_Tick100ms(void)
{
    uint8_t *ipd;
    const char *rx;

    ipd = WiFi_ESP8266_GetIPD(1);
    if (ipd)
        OneNet_MQTT_ProcessDownlink(ipd);

    if (g_retry_div < 10) g_retry_div++;

    if (g_retry_div >= 10)
    {
        g_retry_div = 0;

        switch (g_state)
        {
            case ONENET_STATE_WIFI_JOIN:
                if (WiFi_ESP8266_ConnectAP(WIFI_SSID, WIFI_PWD))
                {
                    dbg_print("[WiFi] GOT IP\r\n");
                    g_state = ONENET_STATE_TCP_CONNECT;
                    g_tcp_backoff_100ms = 0;
                }
                else
                {
                    dbg_print("[WiFi] Join fail\r\n");
                }
                break;

            case ONENET_STATE_TCP_CONNECT:
                if (g_tcp_backoff_100ms)
                {
                    g_tcp_backoff_100ms--;
                    break;
                }
                if (WiFi_ESP8266_TCPConnect(ONENET_HOST, ONENET_PORT))
                {
                    g_tcp_connected = 1;
                    g_state = ONENET_STATE_MQTT_CONNECT;
                    dbg_print("[OneNet] TCP OK\r\n");
                }
                else
                {
                    rx = (const char*)WiFi_ESP8266_DebugGetRxBuf();
                    dbg_printf("[OneNet] TCP retry rx=%s\r\n", rx);
                    if (rx && strstr(rx, "busy") != NULL)
                        g_tcp_backoff_100ms = 30;
                    else
                        g_tcp_backoff_100ms = 10;
                }
                break;

            case ONENET_STATE_MQTT_CONNECT:
                if (OneNet_MQTT_Connect())
                    g_state = ONENET_STATE_SUBSCRIBE;
                break;

            case ONENET_STATE_SUBSCRIBE:
                OneNet_MQTT_SubscribePropertySet();
                g_state = ONENET_STATE_READY;
                break;

            case ONENET_STATE_READY:
            default:
                break;
        }
    }

    g_report_tick++;
}
