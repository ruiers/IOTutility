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

typedef struct _MQTT_Subed_Record_
{
    MemoryByteArray* topic;
    MQTT_Session*    session;
} MQTT_SubedRecord;

typedef struct node_rec
{
    MQTT_SubedRecord data;
    struct node_rec* next;
} MQTT_Sub;

char IsTopicMatch(char* src, char* dst)
{
    int index, src_len, dst_len;

    if ((src == NULL) || (dst == NULL))
        return 0;

    src_len = strlen(src);
    dst_len = strlen(dst);

    if (src_len > dst_len)
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

MQTT_Sub* MQTT_SubNew(char* in_topic, MQTT_Session* in_session)
{
    MQTT_Sub* sub = (MQTT_Sub *) malloc(sizeof(MQTT_Sub));

    (sub->data).session = in_session;
    (sub->data).topic   = (MemoryByteArray *) malloc(sizeof(MemoryByteArray));
    (sub->data).topic->size = strlen(in_topic);
    (sub->data).topic->addr = malloc((sub->data).topic->size);
    memcpy((sub->data).topic->addr, in_topic, (sub->data).topic->size);

    return sub;
}

MQTT_Sub *MQTT_SubIsRecorded(MQTT_Sub *list, char* in_topic, MQTT_Session* in_session)
{
    MQTT_Sub* sub = NULL;

    for (sub = list->next; sub != NULL; sub = sub->next)
    {
        if ((sub->data.session == in_session) && (IsTopicMatch(sub->data.topic->addr, in_topic)))
            break;
    }

    return sub;
}

MQTT_Sub *MQTT_SubHaveTopic(MQTT_Sub *list, char* in_topic)
{
    MQTT_Sub* sub = NULL;

    for (sub = list->next; sub != NULL; sub = sub->next)
    {
        if (IsTopicMatch(sub->data.topic->addr, in_topic))
            break;
    }

    return sub;
}

int MQTT_SubAdd(MQTT_Sub *list, char* in_topic, MQTT_Session* in_session)
{
    MQTT_Sub* sub = NULL;
    MQTT_Sub* record = NULL;

    sub = MQTT_SubIsRecorded(list, in_topic, in_session);

    if (sub)
    {
        return 0;
    }
    else
    {
        record = MQTT_SubNew(in_topic, in_session);
        record->next = list->next;
        list->next  = record;
    }

    return 1;
}

int MQTT_SubRecordDel(MQTT_Sub *list, char* in_topic)
{
    MQTT_Sub* sub = NULL;
    MQTT_Sub* old_sub = list;

    for(sub = list; sub != NULL; sub = sub->next)
    {
        if ((memcmp(sub->data.topic->addr, in_topic, MIN(strlen(in_topic), sub->data.topic->size)) == 0))
        {
            old_sub->next = sub->next;
            free(sub->data.topic->addr);
            free(sub);
        }

        old_sub = sub;
    }
}

MQTT_Sub    *mqttSub = NULL;
MQTT_Server *mqttSrv = NULL;
char buf[512] = { 0 };

void* handleSession(void* arg)
{
    MQTT_Session *session = (MQTT_Session *) arg;
    MQTT_ControlPacket *Packet = NULL;
    MQTT_Sub* sub = NULL;

    while (1)
    {
        Packet = mqttSrv->ACKForSession(mqttSrv, session);

        if ((Packet != NULL) && (Packet->PacketType == PUBLISH))
        {
            for (sub = mqttSub->next; sub != NULL; sub = sub->next)
            {
                if (IsTopicMatch(sub->data.topic->addr, Packet->VariableHeader->addr + 2))
                    session->Session->Send((sub->data).session->Session, Packet->ControlPacket->Memory, Packet->ControlPacket->Length);
            }
        }

        if ((Packet != NULL) && (Packet->PacketType == SUBSCRIBE))
        {
            sub = MQTT_SubIsRecorded(mqttSub, Packet->VariableHeader->addr + 2, session);

            if(sub)
            {

            }
            else
                MQTT_SubAdd(mqttSub, Packet->PayloadStart->addr + 1, session);
        }

        if ((Packet != NULL) && (Packet->PacketType == DISCONNECT))
        {
            continue;
        }

        if (Packet == NULL)
        {
            MQTT_Sub* old_sub = mqttSub;

            for(sub = mqttSub; sub != NULL; sub = sub->next)
            {
                if (sub->data.session == session)
                {
                    old_sub->next = sub->next;
                    free(sub->data.topic->addr);
                    free(sub);
                }

                old_sub = sub;
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
    mqttSub = (MQTT_Sub *) malloc(sizeof(MQTT_Sub));

    mqttSub->next = NULL;

    while (1)
    {
        session = mqttSrv->WaitForSession(mqttSrv);

        if (session)
        {
            printf("%s: connect to %s at %d \n", __func__, inet_ntoa(session->Session->localAddr.sin_addr), ntohs(session->Session->localAddr.sin_port));
            taskCreate(handleSession, session);
        }
    }

    pause();
}

