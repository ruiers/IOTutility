#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "thread.h"
#include "mqttClass.h"
#include "TcpClass.h"
#include "UdpClient.h"
#include <arpa/inet.h>
#include "debug.h"

typedef struct _MQTT_Subed_
{
    MemoryStream topics;
    MQTT_Session* sessions[100];
    int          count;
} MQTT_Subed;

char IsTopicMatch(char* src, char* dst)
{
    int index, src_len, dst_len;

    src_len = strlen(src);
    dst_len = strlen(dst);

    if (dst_len < src_len)
        return 0;

    if ( src[0] != '#' )
    {
        for (index = 0; index < src_len - 1; index++)
        {
            if (src[index] != dst[index])
                return 0;
        }
    }
    else
    {
        for (index = src_len; index > 0; index--)
        {
            if (src[index] != dst[index])
                return 0;
        }
    }

    return 1;
}

MQTT_Subed  *mqttSub = NULL;
MQTT_Server *mqttSrv = NULL;
char buf[512] = { 0 };

void* handleSession(void* arg)
{
    MQTT_Session *session = (MQTT_Session *) arg;
    MQTT_ControlPacket *Packet = NULL;

    while (1)
    {
        Packet = mqttSrv->ACKForSession(mqttSrv, session);

        if ((Packet != NULL) && (Packet->PacketType == PUBLISH))
        {
            log_dbg("topic:%s\nmessage:%s\n", Packet->VariableHeader->addr + 2, Packet->PayloadStart->addr);

            int index = 0;
            MemoryByteArray* array;
            for (array = mqttSub->topics->GetByteArray(mqttSub->topics); array != NULL; array = mqttSub->topics->NextByteArray(array))
            {
                if (IsTopicMatch(array->addr, Packet->VariableHeader->addr + 2))
                {
                    session->Session->Send(mqttSub->sessions[index]->Session, Packet->ControlPacket->Memory, Packet->ControlPacket->Length);
                }

                index++;

            }
        }

        if ((Packet != NULL) && (Packet->PacketType == SUBSCRIBE))
        {
            int index = 0;
            MemoryByteArray* array;
            for (array = mqttSub->topics->GetByteArray(mqttSub->topics); array != NULL; array = mqttSub->topics->NextByteArray(array))
            {
                if (IsTopicMatch(array->addr, Packet->PayloadStart->addr + 1))
                {
                    if (mqttSub->sessions[index] == session)
                    {
                        continue;
                    }
                }

                index++;
            }

            if ((mqttSub->count == 0) || (mqttSub->count == index))
            {
                mqttSub->topics->AddByteArray(mqttSub->topics, Packet->PayloadStart->addr + 1, Packet->PayloadStart->size - 1);
                mqttSub->sessions[mqttSub->count] = session;
                mqttSub->count++;
                log_dbg("add %p", session)
            }
        }

        if ((Packet != NULL) && (Packet->PacketType == DISCONNECT))
        {
            continue;
        }

        if (Packet == NULL)
        {
            int index = 0;
            MemoryByteArray* array;
            for (index = mqttSub->count; index > -1; index--)
            {
                if (mqttSub->sessions[index] == session)
                {
                    mqttSub->sessions[index] = NULL;
                    mqttSub->count--;
                }
            }

            break;
        }

        Packet->ControlPacket->EmptyByteArray(Packet->ControlPacket);
        MemoryStreamDelete(Packet->ControlPacket);
    }
}

void main(int argc, char** argv)
{
    MQTT_Session *session = NULL;

    mqttSrv = MQTT_ServerCreate("127.0.0.1", 1888);
    mqttSub = (MQTT_Subed *) malloc(sizeof(MQTT_Subed));

    mqttSub->topics = MemoryStreamCreate();
    mqttSub->count  = 0;

    while (1)
    {
        session = mqttSrv->WaitForSession(mqttSrv);
        printf("%s: connect to %s at %d \n", __func__, inet_ntoa(session->Session->localAddr.sin_addr), ntohs(session->Session->localAddr.sin_port));
        taskCreate(handleSession, session);
    }

    pause();
}

