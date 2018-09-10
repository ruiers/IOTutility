#ifndef __INC_LINUX_MQTTCLASS__
#define __INC_LINUX_MQTTCLASS__

#include "linux/MemoryStream.h"

#define PROTOCOL_NAME_v31 "MQIsdp"
#define PROTOCOL_VERSION_v31 3

#define PROTOCOL_NAME_v311 "MQTT"
#define PROTOCOL_VERSION_v311 4

/* MQTT specification restricts client ids to a maximum of 23 characters */
#define MOSQ_MQTT_ID_MAX_LENGTH 23

#define CONNACK_ACCEPTED 0
#define CONNACK_REFUSED_PROTOCOL_VERSION 1
#define CONNACK_REFUSED_IDENTIFIER_REJECTED 2
#define CONNACK_REFUSED_SERVER_UNAVAILABLE 3
#define CONNACK_REFUSED_BAD_USERNAME_PASSWORD 4
#define CONNACK_REFUSED_NOT_AUTHORIZED 5

#define MQTT_MAX_PAYLOAD 268435455

/* Message types */
#define CONNECT 0x10
#define CONNACK 0x20
#define PUBLISH 0x30
#define PUBACK 0x40
#define PUBREC 0x50
#define PUBREL 0x60
#define PUBCOMP 0x70
#define SUBSCRIBE 0x80
#define SUBACK 0x90
#define UNSUBSCRIBE 0xA0
#define UNSUBACK 0xB0
#define PINGREQ 0xC0
#define PINGRESP 0xD0
#define DISCONNECT 0xE0

typedef struct FixedHeader
{
    char type_and_flag;
    char remaining_length[4];
} FixedHeader;

typedef struct VariableHeader_Connect
{
    short protocol_length;
    char  protocol_name[4];
    char  protocol_level;
    char  connect_flags;
    short keep_alive;
} VariableHeader_Connect;

typedef struct Payload
{
    short length;
    char  message[0];
} Payload;

typedef struct mqtt_control_packet
{
    int              PacketType;
    int              PacketLength;
    char*            PacketData;

    MemoryStream     ControlPacket;
    MemoryByteArray* FixedHeader;
    MemoryByteArray* VariableHeader;
    MemoryByteArray* PayloadStart;
} MQTT_ControlPacket;

MQTT_ControlPacket* MQTT_ControlPacketCreate(int PacketType);
char* MQTT_ControlPacketGetPacketData(MQTT_ControlPacket* this);
int MQTT_ControlPacketSetTopic(MQTT_ControlPacket* this, char* topic_string, int topic_length);
int MQTT_ControlPacketSetMessage(MQTT_ControlPacket* this, char* msg_string, int msg_length);
#endif