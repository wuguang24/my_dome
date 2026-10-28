#include "stm32f10x.h"
#include "esp8266.h"
#include "onenet.h"
#include "MqttKit.h"
#include "Delay.h"

#include <string.h>
#include <stdio.h>

#include "aliyun_conf.h"
 #include "dbg.h"

extern volatile uint32_t system_time_ms;

static char onenet_rx_payload[512];
static volatile uint8_t onenet_rx_ready = 0;

static volatile uint8_t s_mqtt_connected = 0;
static volatile uint8_t s_subscribed = 0;
static const char *s_topic_set = NULL;
static uint32_t s_next_action_ms = 0;
static uint32_t s_last_ping_ms = 0;
static uint8_t s_wait_pingresp = 0;
static uint8_t s_last_tcp_connected = 0;

const char *OneNet_GetRxPayload(void)
{
    return onenet_rx_ready ? (const char *)onenet_rx_payload : NULL;
}

void OneNet_ClearRxPayload(void)
{
    onenet_rx_ready = 0;
    onenet_rx_payload[0] = 0;
}

void OneNet_ConfigSubscribeTopic(const char *topic_set)
{
    s_topic_set = topic_set;
}

uint8_t OneNet_IsMqttConnected(void)
{
    return s_mqtt_connected;
}

uint8_t OneNet_IsSubscribed(void)
{
    return s_subscribed;
}

void OneNet_Service(void)
{
    uint32_t now = system_time_ms;
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
    const char *topics[1];

    if(s_last_tcp_connected != ESP8266_IsTcpConnected())
    {
        s_last_tcp_connected = ESP8266_IsTcpConnected();
        DBG_PRINTF("[MQTT] tcp=%d\r\n", (int)s_last_tcp_connected);
    }

    if(!ESP8266_IsTcpConnected())
    {
        s_mqtt_connected = 0;
        s_subscribed = 0;
        s_wait_pingresp = 0;
        s_last_ping_ms = now;
        s_next_action_ms = now + 500;
        return;
    }

    if((int32_t)(now - s_next_action_ms) < 0)
        return;

    if(!s_mqtt_connected)
    {
        DBG_PRINTF("[MQTT] CONNECT...\r\n");
        if(MQTT_PacketConnect(ALIYUN_MQTT_USER, ALIYUN_MQTT_PASS, ALIYUN_MQTT_CLIENTID,
                              256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
        {
            ESP8266_SendData(mqttPacket._data, mqttPacket._len);
            MQTT_DeleteBuffer(&mqttPacket);
        }
        s_next_action_ms = now + 2000;
        return;
    }

    if(!s_subscribed && s_topic_set != NULL)
    {
        DBG_PRINTF("[MQTT] SUBSCRIBE %s\r\n", s_topic_set);
        topics[0] = s_topic_set;
        if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL2, topics, 1, &mqttPacket) == 0)
        {
            ESP8266_SendData(mqttPacket._data, mqttPacket._len);
            MQTT_DeleteBuffer(&mqttPacket);
        }
        s_next_action_ms = now + 2000;
        return;
    }

    // Keepalive ping (non-blocking)
    if(s_wait_pingresp)
    {
        if((now - s_last_ping_ms) > 5000)
        {
            DBG_PRINTF("[MQTT] PING TIMEOUT -> reconnect\r\n");
            // ping timeout -> force reconnect
            s_mqtt_connected = 0;
            s_subscribed = 0;
            s_wait_pingresp = 0;
            s_next_action_ms = now + 500;
            return;
        }
    }
    else
    {
        if((now - s_last_ping_ms) > 30000)
        {
            if(MQTT_PacketPing(&mqttPacket) == 0)
            {
                DBG_PRINTF("[MQTT] PING\r\n");
                ESP8266_SendData(mqttPacket._data, mqttPacket._len);
                MQTT_DeleteBuffer(&mqttPacket);
                s_last_ping_ms = now;
                s_wait_pingresp = 1;
            }
        }
    }

    s_next_action_ms = now + 200;
}

_Bool OneNet_DevLink(void)
{
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

    unsigned char *dataPtr;
    _Bool status = 1;

    if(MQTT_PacketConnect(ALIYUN_MQTT_USER, ALIYUN_MQTT_PASS, ALIYUN_MQTT_CLIENTID,
                          256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
    {
        ESP8266_SendData(mqttPacket._data, mqttPacket._len);

        dataPtr = ESP8266_GetIPD(250);
        if(dataPtr != NULL)
        {
            if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
            {
                if(MQTT_UnPacketConnectAck(dataPtr) == 0) status = 0;
            }
        }

        MQTT_DeleteBuffer(&mqttPacket);
    }

    return status;
}

void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

    if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL2, topics, topic_cnt, &mqttPacket) == 0)
    {
        ESP8266_SendData(mqttPacket._data, mqttPacket._len);
        MQTT_DeleteBuffer(&mqttPacket);
    }
}

void OneNet_Publish(const char *topic, const char *msg)
{
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

    if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL2, 0, 1, &mqttPacket) == 0)
    {
        ESP8266_SendData(mqttPacket._data, mqttPacket._len);
        MQTT_DeleteBuffer(&mqttPacket);
    }
}

void OneNet_RevPro(unsigned char *cmd)
{
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

    char *req_payload = NULL;
    char *cmdid_topic = NULL;

    unsigned short topic_len = 0;
    unsigned short req_len = 0;

    unsigned char type = 0;
    unsigned char qos = 0;
    static unsigned short pkt_id = 0;

    short result = 0;

    type = MQTT_UnPacketRecv(cmd);
    switch(type)
    {
        case MQTT_PKT_CONNACK:
            if(MQTT_UnPacketConnectAck(cmd) == 0)
            {
                DBG_PRINTF("[MQTT] CONNACK OK\r\n");
                s_mqtt_connected = 1;
                s_subscribed = 0;
            }
            else
            {
                DBG_PRINTF("[MQTT] CONNACK FAIL\r\n");
            }
            break;

        case MQTT_PKT_CMD:
            result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);
            if(result == 0)
            {
                if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)
                {
                    ESP8266_SendData(mqttPacket._data, mqttPacket._len);
                    MQTT_DeleteBuffer(&mqttPacket);
                }
            }
            break;

        case MQTT_PKT_PUBLISH:
            result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
            if(result == 0)
            {
                if(req_payload != NULL)
                {
                    size_t n = (req_len < sizeof(onenet_rx_payload) - 1) ? req_len : (sizeof(onenet_rx_payload) - 1);
                    memcpy(onenet_rx_payload, req_payload, n);
                    onenet_rx_payload[n] = 0;
                    onenet_rx_ready = 1;
                    DBG_PRINTF("[MQTT] DOWNLINK len=%u\r\n", (unsigned int)n);
                }

                switch(qos)
                {
                    case 1:
                        if(MQTT_PacketPublishAck(pkt_id, &mqttPacket) == 0)
                        {
                            ESP8266_SendData(mqttPacket._data, mqttPacket._len);
                            MQTT_DeleteBuffer(&mqttPacket);
                        }
                        break;

                    case 2:
                        if(MQTT_PacketPublishRec(pkt_id, &mqttPacket) == 0)
                        {
                            ESP8266_SendData(mqttPacket._data, mqttPacket._len);
                            MQTT_DeleteBuffer(&mqttPacket);
                        }
                        break;

                    default:
                        break;
                }
            }
            break;

        case MQTT_PKT_PUBACK:
            // QoS1 publish ack for outgoing publish
            (void)MQTT_UnPacketPublishAck(cmd);
            break;

        case MQTT_PKT_PUBREC:
            // QoS2 publish rec for outgoing publish, reply rel
            if(MQTT_UnPacketPublishRec(cmd) == 0)
            {
                if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
                {
                    ESP8266_SendData(mqttPacket._data, mqttPacket._len);
                    MQTT_DeleteBuffer(&mqttPacket);
                }
            }
            break;

        case MQTT_PKT_PUBREL:
            // QoS2 flow when we received publish and replied rec, now reply comp
            if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
            {
                if(MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
                {
                    ESP8266_SendData(mqttPacket._data, mqttPacket._len);
                    MQTT_DeleteBuffer(&mqttPacket);
                }
            }
            break;

        case MQTT_PKT_PUBCOMP:
            // QoS2 publish completed
            (void)MQTT_UnPacketPublishComp(cmd);
            break;

        case MQTT_PKT_SUBACK:
            if(MQTT_UnPacketSubscribe(cmd) == 0)
            {
                DBG_PRINTF("[MQTT] SUBACK OK\r\n");
                s_subscribed = 1;
                s_last_ping_ms = system_time_ms;
                s_wait_pingresp = 0;
            }
            else
            {
                DBG_PRINTF("[MQTT] SUBACK FAIL\r\n");
            }
            break;

        case MQTT_PKT_PINGRESP:
            DBG_PRINTF("[MQTT] PINGRESP\r\n");
            s_wait_pingresp = 0;
            s_last_ping_ms = system_time_ms;
            break;

        case MQTT_PKT_DISCONNECT:
            DBG_PRINTF("[MQTT] DISCONNECT\r\n");
            s_mqtt_connected = 0;
            s_subscribed = 0;
            s_wait_pingresp = 0;
            s_next_action_ms = system_time_ms + 500;
            break;

        case MQTT_PKT_UNSUBACK:
            (void)MQTT_UnPacketUnSubscribe(cmd);
            break;
        default:
            break;
    }

    ESP8266_Clear();

    if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
    {
        if(cmdid_topic) MQTT_FreeBuffer(cmdid_topic);
        if(req_payload) MQTT_FreeBuffer(req_payload);
    }
}
