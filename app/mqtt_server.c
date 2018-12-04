#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "thread.h"
#include "mqttClass.h"
#include "TcpClass.h"
#include "UdpClient.h"
#include <arpa/inet.h>
#include "common.h"
#include "debug.h"

/* Error values */
enum mosq_err_t
{
    MOSQ_ERR_CONN_PENDING = -1,
    MOSQ_ERR_SUCCESS = 0,
    MOSQ_ERR_NOMEM = 1,
    MOSQ_ERR_PROTOCOL = 2,
    MOSQ_ERR_INVAL = 3,
    MOSQ_ERR_NO_CONN = 4,
    MOSQ_ERR_CONN_REFUSED = 5,
    MOSQ_ERR_NOT_FOUND = 6,
    MOSQ_ERR_CONN_LOST = 7,
    MOSQ_ERR_TLS = 8,
    MOSQ_ERR_PAYLOAD_SIZE = 9,
    MOSQ_ERR_NOT_SUPPORTED = 10,
    MOSQ_ERR_AUTH = 11,
    MOSQ_ERR_ACL_DENIED = 12,
    MOSQ_ERR_UNKNOWN = 13,
    MOSQ_ERR_ERRNO = 14,
    MOSQ_ERR_EAI = 15,
    MOSQ_ERR_PROXY = 16,
    MOSQ_ERR_PLUGIN_DEFER = 17,
    MOSQ_ERR_MALFORMED_UTF8 = 18,
    MOSQ_ERR_KEEPALIVE = 19,
    MOSQ_ERR_LOOKUP = 20,
};

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

/* Does a topic match a subscription? */
int mosquitto_topic_matches_sub2(const char *sub, size_t sublen, const char *topic, size_t topiclen, bool *result)
{
    int spos, tpos;
    bool multilevel_wildcard = false;

    if(!result) return MOSQ_ERR_INVAL;
    *result = false;

    if(!sub || !topic)
    {
        return MOSQ_ERR_INVAL;
    }

    if(!sublen || !topiclen)
    {
        *result = false;
        return MOSQ_ERR_INVAL;
    }

    if(sublen && topiclen)
    {
        if((sub[0] == '$' && topic[0] != '$')
                || (topic[0] == '$' && sub[0] != '$'))
        {

            return MOSQ_ERR_SUCCESS;
        }
    }

    spos = 0;
    tpos = 0;

    while(spos < sublen && tpos <= topiclen)
    {
        if(sub[spos] == topic[tpos])
        {
            if(tpos == topiclen-1)
            {
                /* Check for e.g. foo matching foo/# */
                if(spos == sublen-3
                        && sub[spos+1] == '/'
                        && sub[spos+2] == '#')
                {
                    *result = true;
                    multilevel_wildcard = true;
                    return MOSQ_ERR_SUCCESS;
                }
            }
            spos++;
            tpos++;
            if(spos == sublen && tpos == topiclen)
            {
                *result = true;
                return MOSQ_ERR_SUCCESS;
            }
            else if(tpos == topiclen && spos == sublen-1 && sub[spos] == '+')
            {
                if(spos > 0 && sub[spos-1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                spos++;
                *result = true;
                return MOSQ_ERR_SUCCESS;
            }
        }
        else
        {
            if(sub[spos] == '+')
            {
                /* Check for bad "+foo" or "a/+foo" subscription */
                if(spos > 0 && sub[spos-1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                /* Check for bad "foo+" or "foo+/a" subscription */
                if(spos < sublen-1 && sub[spos+1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                spos++;
                while(tpos < topiclen && topic[tpos] != '/')
                {
                    tpos++;
                }
                if(tpos == topiclen && spos == sublen)
                {
                    *result = true;
                    return MOSQ_ERR_SUCCESS;
                }
            }
            else if(sub[spos] == '#')
            {
                if(spos > 0 && sub[spos-1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                multilevel_wildcard = true;
                if(spos+1 != sublen)
                {
                    return MOSQ_ERR_INVAL;
                }
                else
                {
                    *result = true;
                    return MOSQ_ERR_SUCCESS;
                }
            }
            else
            {
                /* Check for e.g. foo/bar matching foo/+/# */
                if(spos > 0
                        && spos+2 == sublen
                        && tpos == topiclen
                        && sub[spos-1] == '+'
                        && sub[spos] == '/'
                        && sub[spos+1] == '#')
                {
                    *result = true;
                    multilevel_wildcard = true;
                    return MOSQ_ERR_SUCCESS;
                }
                return MOSQ_ERR_SUCCESS;
            }
        }
    }
    if(multilevel_wildcard == false && (tpos < topiclen || spos < sublen))
    {
        *result = false;
    }

    return MOSQ_ERR_SUCCESS;
}

bool IsTopicMatch(const char *sub, const char *topic)
{
    int slen, tlen, rerr;
    bool result = false;

    if(!sub || !topic)
    {
        return false;
    }

    slen = strlen(sub);
    tlen = strlen(topic);

    rerr =  mosquitto_topic_matches_sub2(sub, slen, topic, tlen, &result);

    if ((rerr == 0) && (result == true))
        return true;

    else
    {
        return false;
    }

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
    bool matched = false;

    for (sub = list->next; sub != NULL; sub = sub->next)
    {
        if (IsTopicMatch(sub->data.topic->addr, in_topic))
            return NULL;
        else
        {
            if (matched == true)
                break;
        }
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

    if (mqttSrv == NULL)
        return;

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

