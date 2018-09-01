#include <stdio.h>
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

    void* fixedHeader = NULL;
    void* variableHeader = NULL;
    void* PayloadStart = NULL;

    packet->PacketType = PacketType;

    switch (packet->PacketType)
    {
    case CONNECT:
        variableHeader = (VariableHeader_Connect*) calloc(1, sizeof(VariableHeader_Connect));
        ((VariableHeader_Connect*) variableHeader)->protocol_length = htons(sizeof(PROTOCOL_NAME_v311));
        memcpy(((VariableHeader_Connect*) variableHeader)->protocol_name, PROTOCOL_NAME_v311, sizeof(PROTOCOL_NAME_v311));
        ((VariableHeader_Connect*) variableHeader)->connect_flags = 2;
        ((VariableHeader_Connect*) variableHeader)->keep_alive = 60;


        PayloadStart = (Payload*) calloc(1, 23 + sizeof(short));
        ((Payload*) PayloadStart)->length = client_id_generate(((Payload*) PayloadStart)->message, "mosqpub");

        fixedHeader = (FixedHeader*) calloc(1, 5);
        ((FixedHeader*) fixedHeader)->type_and_flag = packet->PacketType;
        ((FixedHeader*) fixedHeader)->remaining_length[0] = sizeof(VariableHeader_Connect) + ((Payload*) PayloadStart)->length + sizeof(short);

        packet->FixedHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) fixedHeader, 2);
        packet->VariableHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) variableHeader, sizeof(VariableHeader_Connect));
        packet->PayloadStart = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) PayloadStart, ((Payload*) PayloadStart)->length + sizeof(short));

        break;
    case PUBLISH:
        break;
    default:
        break;
    }

    return packet;
}

char* MQTT_ControlPacketGetPacketData(MQTT_ControlPacket* this)
{
    MemoryByteArray* array = this->ControlPacket->GetByteArray(this->ControlPacket);
    int pos = 0;
    this->PacketLength = this->FixedHeader->size + this->VariableHeader->size + this->PayloadStart->size;
    this->PacketData = calloc(1, this->PacketLength);

    while(array != NULL)
    {
        memcpy(this->PacketData + pos, array->addr, array->size);
        pos += array->size;
        array = this->ControlPacket->NextByteArray(array);
    }

    return this->PacketData;
}
