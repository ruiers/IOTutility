#include <stdio.h>
#include <netinet/in.h>
#include "linux/mqttClass.h"

int client_id_generate(char* client_id, const char *id_base)
{
    int len;
    char hostname[256];

    if (client_id == NULL)
    {
        client_id = malloc(len);
        if(!client_id)
        {
            return -1;
        }
    }
    else
    {
        hostname[0] = '\0';
        gethostname(hostname, 256);
        hostname[255] = '\0';
        len = strlen(id_base) + strlen("|-") + 6 + strlen(hostname);
        snprintf(client_id, len, "%s|%d-%s", id_base, getpid(), hostname);

        if(strlen(client_id) > MOSQ_MQTT_ID_MAX_LENGTH)
        {
            /* Enforce maximum client id length of 23 characters */
            client_id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
        }
    }

    return strlen(client_id);
}

MQTT_ControlPacket* MQTT_ControlPacketCreate(int PacketType)
{
    MQTT_ControlPacket* packet = calloc(1, sizeof(MQTT_ControlPacket));

    packet->ControlPacket = MemoryStreamCreate();

    int   length_of_payload = 0;
    void* fixedHeader = NULL;
    void* variableHeader = NULL;
    void* PayloadStart = NULL;

    packet->PacketType = PacketType;

    switch (packet->PacketType)
    {
    case CONNECT:
        variableHeader = (VariableHeader_Connect*) calloc(1, sizeof(VariableHeader_Connect));
        ((VariableHeader_Connect*) variableHeader)->protocol_length = htons(sizeof(PROTOCOL_NAME_v311) - 1);
        memcpy(((VariableHeader_Connect*) variableHeader)->protocol_name, PROTOCOL_NAME_v311, sizeof(PROTOCOL_NAME_v311) - 1);
        ((VariableHeader_Connect*) variableHeader)->protocol_level = PROTOCOL_VERSION_v311;
        ((VariableHeader_Connect*) variableHeader)->connect_flags = 2;
        ((VariableHeader_Connect*) variableHeader)->keep_alive = htons(60);


        PayloadStart = (Payload*) calloc(1, 23 + sizeof(short));
        length_of_payload = client_id_generate(((Payload*) PayloadStart)->message, "mosqpub");
        ((Payload*) PayloadStart)->length = htons(length_of_payload);

        fixedHeader = (FixedHeader*) calloc(1, 5);
        ((FixedHeader*) fixedHeader)->type_and_flag = packet->PacketType;
        ((FixedHeader*) fixedHeader)->remaining_length[0] = sizeof(VariableHeader_Connect) + (length_of_payload + sizeof(short));

        packet->FixedHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) fixedHeader, 2);
        packet->VariableHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) variableHeader, sizeof(VariableHeader_Connect));
        packet->PayloadStart = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) PayloadStart, length_of_payload + sizeof(short));

        break;
    case PUBLISH:
        fixedHeader = (FixedHeader*) calloc(1, 5);
        ((FixedHeader*) fixedHeader)->type_and_flag = packet->PacketType;
        ((FixedHeader*) fixedHeader)->remaining_length[0] = 0;

        packet->FixedHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) fixedHeader, 2);
        break;
    case DISCONNECT:
        fixedHeader = (FixedHeader*) calloc(1, 5);
        ((FixedHeader*) fixedHeader)->type_and_flag = packet->PacketType;
        ((FixedHeader*) fixedHeader)->remaining_length[0] = 0;

        packet->FixedHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) fixedHeader, 2);
    default:
        break;
    }

    return packet;
}

char* MQTT_ControlPacketGetPacketData(MQTT_ControlPacket* this)
{
    MemoryByteArray* array = this->ControlPacket->GetByteArray(this->ControlPacket);
    int pos = 0;
    this->PacketLength = this->ControlPacket->Length;
    this->PacketData = calloc(1, this->PacketLength);

    while(array != NULL)
    {
        memcpy(this->PacketData + pos, array->addr, array->size);
        pos += array->size;
        array = this->ControlPacket->NextByteArray(array);
    }

    return this->PacketData;
}

int MQTT_ControlPacketSetTopic(MQTT_ControlPacket* this, char* topic_string, int topic_length)
{
    FixedHeader* fixedHeader = (FixedHeader*) this->FixedHeader->addr;
    Payload* topic_payload = calloc(1, sizeof(short) + topic_length);

    topic_payload->length = htons(topic_length);
    memcpy(topic_payload->message, topic_string, topic_length);
    this->VariableHeader = this->ControlPacket->AddByteArray(this->ControlPacket, (char *) topic_payload, sizeof(short) + topic_length);

    ((FixedHeader*) fixedHeader)->remaining_length[0] += this->VariableHeader->size;
}

int MQTT_ControlPacketSetMessage(MQTT_ControlPacket* this, char* msg_string, int msg_length)
{
    FixedHeader* fixedHeader = (FixedHeader*) this->FixedHeader->addr;

    this->PayloadStart = this->ControlPacket->AddByteArray(this->ControlPacket, (char *) msg_string, msg_length);
    ((FixedHeader*) fixedHeader)->remaining_length[0] += this->PayloadStart->size;
}

int MQTT_ConnectionConnect(MQTT_Connection* this)
{
    MQTT_ControlPacket*  mqttConnect = MQTT_ControlPacketCreate(CONNECT);
    MQTT_ACKPacket       mqttACK;

    this->Status = STA_WAITING_ACK;
    MQTT_ControlPacketGetPacketData(mqttConnect);
    this->Connection->Send(this->Connection, mqttConnect->PacketData, mqttConnect->PacketLength);
    this->Connection->Receive(this->Connection, (char *) &mqttACK);

    if ((mqttACK.type_and_flag == CONNACK) && (mqttACK.ack_code[1] == CONNACK_ACCEPTED))
        this->Status = STA_CONNECTED;
    else
        this->Status = STA_CONNECT_NO_ACK;
}

int MQTT_ConnectionDisonnect(MQTT_Connection* this)
{
    MQTT_ControlPacket*  mqttDisconnect = MQTT_ControlPacketCreate(DISCONNECT);
    MQTT_ControlPacketGetPacketData(mqttDisconnect);
    this->Connection->Send(this->Connection, mqttDisconnect->PacketData, mqttDisconnect->PacketLength);
}

int MQTT_ConnectionPublish(MQTT_Connection* this, char* topic, char* message, int length)
{
    MQTT_ControlPacket*  mqttPublish = MQTT_ControlPacketCreate(PUBLISH);
    MQTT_ACKPacket       mqttACK;

    if (this->Status != STA_CONNECTED)
        return -1;

    this->Status = STA_WAITING_ACK;
    MQTT_ControlPacketSetTopic(mqttPublish, topic, strlen(topic));
    MQTT_ControlPacketSetMessage(mqttPublish, message, length);
    MQTT_ControlPacketGetPacketData(mqttPublish);
    this->Connection->Send(this->Connection, mqttPublish->PacketData, mqttPublish->PacketLength);
    //this->Connection->Receive(this->Connection, (char *) &mqttACK);

    this->Status = STA_CONNECTED;
}

MQTT_Connection* MQTT_ConnectionCreate(char* ipStr, int portNum)
{
    MQTT_Connection* connection = calloc(1, sizeof(MQTT_Connection));

    connection->ServerPortNumber = portNum;
    strncpy(connection->ServerIPString, ipStr, strlen(ipStr));

    connection->Connection = tcpClientCreate(connection->ServerIPString, connection->ServerPortNumber);

    connection->Connect    = MQTT_ConnectionConnect;
    connection->Disconnect = MQTT_ConnectionDisonnect;
    connection->Publish    = MQTT_ConnectionPublish;

    connection->Status     = STA_CREATED;
    return connection;
}
