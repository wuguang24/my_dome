#include "MqttKit.h"

#include <string.h>
#include <stdio.h>

static void MQTT_NewBuffer(MQTT_PACKET_STRUCTURE *mqttPacket, uint32 size)
{
	uint32 i = 0;

	if (mqttPacket->_data == NULL)
	{
		mqttPacket->_memFlag = MEM_FLAG_ALLOC;
		mqttPacket->_data = (uint8 *)MQTT_MallocBuffer(size);
		if (mqttPacket->_data != NULL)
		{
			mqttPacket->_len = 0;
			mqttPacket->_size = size;
			for (; i < mqttPacket->_size; i++) mqttPacket->_data[i] = 0;
		}
	}
	else
	{
		mqttPacket->_memFlag = MEM_FLAG_STATIC;
		for (; i < mqttPacket->_size; i++) mqttPacket->_data[i] = 0;
		mqttPacket->_len = 0;
		if (mqttPacket->_size < size) mqttPacket->_data = NULL;
	}
}

void MQTT_DeleteBuffer(MQTT_PACKET_STRUCTURE *mqttPacket)
{
	if (mqttPacket->_memFlag == MEM_FLAG_ALLOC)
		MQTT_FreeBuffer(mqttPacket->_data);

	mqttPacket->_data = NULL;
	mqttPacket->_len = 0;
	mqttPacket->_size = 0;
	mqttPacket->_memFlag = MEM_FLAG_NULL;
}

static int32 MQTT_DumpLength(size_t len, uint8 *buf)
{
	int32 i = 0;
	for (i = 1; i <= 4; ++i)
	{
		*buf = len % 128;
		len >>= 7;
		if (len > 0)
		{
			*buf |= 128;
			++buf;
		}
		else
		{
			return i;
		}
	}
	return -1;
}

static void MQTT_WriteInt(uint8 **pptr, uint16 anInt)
{
	**pptr = MOSQ_MSB(anInt);
	(*pptr)++;
	**pptr = MOSQ_LSB(anInt);
	(*pptr)++;
}

static void MQTT_WriteString(uint8 **pptr, const int8 *string)
{
	uint16 len = (uint16)strlen((const char*)string);
	MQTT_WriteInt(pptr, len);
	memcpy(*pptr, string, len);
	*pptr += len;
}

static uint32 MQTT_GetPacketLength(uint32 length)
{
	uint32 len = 1;
	while (length > 127)
	{
		length >>= 7;
		len++;
	}
	return len;
}

uint8 MQTT_UnPacketRecv(uint8 *dataPtr)
{
	if (dataPtr == NULL) return 0;
	return (dataPtr[0] >> 4) & 0x0F;
}

static uint8 MQTT_UnPacketRemainingLen(uint8 *buf, uint32 *len)
{
	uint32 multiplier = 1;
	uint32 value = 0;
	uint8 i = 0;
	uint8 encodedByte;

	do
	{
		encodedByte = buf[i + 1];
		value += (encodedByte & 127) * multiplier;
		multiplier *= 128;
		i++;
		if (i > 4) return 0;
	} while ((encodedByte & 128) != 0);

	*len = value;
	return i;
}

uint8 MQTT_UnPacketConnectAck(uint8 *rev_data)
{
	uint32 remain_len = 0;
	uint8 len_bytes = MQTT_UnPacketRemainingLen(rev_data, &remain_len);
	if (len_bytes == 0) return 0xFF;
	if (remain_len < 2) return 0xFF;
	return rev_data[1 + len_bytes + 1];
}

uint8 MQTT_PacketConnect(const int8 *user, const int8 *password, const int8 *devid,
						uint16 cTime, uint1 clean_session, uint1 qos,
						const int8 *will_topic, const int8 *will_msg, int32 will_retain,
						MQTT_PACKET_STRUCTURE *mqttPacket)
{
	uint8 flags = 0;
	uint32 len = 0;
	uint8 *ptr;
	uint8 rem_len_bytes;
	uint8 tmp_len[4];
	uint8 tmp_len_size;

	(void)qos;
	(void)will_topic;
	(void)will_msg;
	(void)will_retain;

	if (clean_session) flags |= MQTT_CONNECT_CLEAN_SESSION;
	if (user != NULL) flags |= MQTT_CONNECT_USER_NAME;
	if (password != NULL) flags |= MQTT_CONNECT_PASSORD;

	len = 10;
	len += 2 + (uint16)strlen((const char*)devid);
	if (user) len += 2 + (uint16)strlen((const char*)user);
	if (password) len += 2 + (uint16)strlen((const char*)password);

	rem_len_bytes = MQTT_GetPacketLength(len);
	tmp_len_size = (uint8)MQTT_DumpLength(len, tmp_len);

	MQTT_NewBuffer(mqttPacket, 1 + rem_len_bytes + len);
	if (mqttPacket->_data == NULL) return 1;

	ptr = mqttPacket->_data;
	*ptr++ = (uint8)(MQTT_PKT_CONNECT << 4);
	memcpy(ptr, tmp_len, tmp_len_size);
	ptr += tmp_len_size;

	MQTT_WriteString(&ptr, (const int8*)"MQTT");
	*ptr++ = 0x04;
	*ptr++ = flags;
	MQTT_WriteInt(&ptr, cTime);

	MQTT_WriteString(&ptr, devid);
	if (user) MQTT_WriteString(&ptr, user);
	if (password) MQTT_WriteString(&ptr, password);

	mqttPacket->_len = (uint32)(ptr - mqttPacket->_data);
	return 0;
}

uint1 MQTT_PacketSaveData(const int8 *devid, int16 send_len, int8 *type_bin_head, uint8 type, MQTT_PACKET_STRUCTURE *mqttPacket)
{
	(void)devid;
	(void)send_len;
	(void)type_bin_head;
	(void)type;
	(void)mqttPacket;
	return 1;
}

uint8 MQTT_PacketSubscribe(uint16 pkt_id, enum MqttQosLevel qos, const int8 *topics[], uint8 topics_cnt, MQTT_PACKET_STRUCTURE *mqttPacket)
{
	uint8 *ptr;
	uint32 len = 2;
	uint8 tmp_len[4];
	uint8 tmp_len_size;

	for (uint8 i = 0; i < topics_cnt; i++)
		len += 2 + (uint16)strlen((const char*)topics[i]) + 1;

	tmp_len_size = (uint8)MQTT_DumpLength(len, tmp_len);
	MQTT_NewBuffer(mqttPacket, 1 + tmp_len_size + len);
	if (mqttPacket->_data == NULL) return 1;

	ptr = mqttPacket->_data;
	*ptr++ = (uint8)((MQTT_PKT_SUBSCRIBE << 4) | 0x02);
	memcpy(ptr, tmp_len, tmp_len_size);
	ptr += tmp_len_size;

	MQTT_WriteInt(&ptr, pkt_id);

	for (uint8 i = 0; i < topics_cnt; i++)
	{
		MQTT_WriteString(&ptr, topics[i]);
		*ptr++ = (uint8)qos;
	}

	mqttPacket->_len = (uint32)(ptr - mqttPacket->_data);
	return 0;
}

uint8 MQTT_UnPacketPublish(uint8 *rev_data, int8 **topic, uint16 *topic_len, int8 **payload, uint16 *payload_len, uint8 *qos, uint16 *pkt_id)
{
	uint32 remain_len = 0;
	uint8 len_bytes = MQTT_UnPacketRemainingLen(rev_data, &remain_len);
	uint8 *ptr;
	uint16 tlen;

	*qos = (rev_data[0] & 0x06) >> 1;

	ptr = rev_data + 1 + len_bytes;
	if (remain_len < 2) return 1;

	tlen = (uint16)((ptr[0] << 8) | ptr[1]);
	ptr += 2;

	*topic = (int8*)ptr;
	*topic_len = tlen;
	ptr += tlen;

	if (*qos > 0)
	{
		*pkt_id = (uint16)((ptr[0] << 8) | ptr[1]);
		ptr += 2;
	}

	*payload = (int8*)ptr;
	*payload_len = (uint16)(remain_len - 2 - tlen - ((*qos > 0) ? 2 : 0));
	return 0;
}

uint8 MQTT_UnPacketCmd(uint8 *rev_data, int8 **cmdid, int8 **req, uint16 *req_len)
{
	(void)rev_data;
	(void)cmdid;
	(void)req;
	(void)req_len;
	return 1;
}

uint1 MQTT_PacketCmdResp(const int8 *cmdid, const int8 *req, MQTT_PACKET_STRUCTURE *mqttPacket)
{
	(void)cmdid;
	(void)req;
	(void)mqttPacket;
	return 1;
}

uint8 MQTT_PacketPublish(uint16 pkt_id, const int8 *topic,
							const int8 *payload, uint32 payload_len,
							enum MqttQosLevel qos, int32 retain, int32 own,
							MQTT_PACKET_STRUCTURE *mqttPacket)
{
	uint8 *ptr;
	uint32 len;
	uint8 tmp_len[4];
	uint8 tmp_len_size;

	(void)retain;
	(void)own;

	len = 2 + (uint16)strlen((const char*)topic) + payload_len + ((qos > 0) ? 2 : 0);
	tmp_len_size = (uint8)MQTT_DumpLength(len, tmp_len);

	MQTT_NewBuffer(mqttPacket, 1 + tmp_len_size + len);
	if (mqttPacket->_data == NULL) return 1;

	ptr = mqttPacket->_data;
	*ptr++ = (uint8)((MQTT_PKT_PUBLISH << 4) | ((uint8)qos << 1));
	memcpy(ptr, tmp_len, tmp_len_size);
	ptr += tmp_len_size;

	MQTT_WriteString(&ptr, topic);
	if (qos > 0) MQTT_WriteInt(&ptr, pkt_id);

	memcpy(ptr, payload, payload_len);
	ptr += payload_len;

	mqttPacket->_len = (uint32)(ptr - mqttPacket->_data);
	return 0;
}

uint1 MQTT_UnPacketPublishAck(uint8 *rev_data)
{
	(void)rev_data;
	return 1;
}
