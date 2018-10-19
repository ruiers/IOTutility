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

MQTT_Server *mqttSrv = NULL;
char buf[512] = { 0 };

void hexdump(char *data, int len)
{
    int i = 0, value = 0;
    for (i = 0; i < len; i++)
    {
        value = *((unsigned char *) data + i);

        if ((i) % 2 == 0)
        {
            printf(" ");
        }

        if ((i) % 16 == 0)
        {
            printf("\n");
            printf("%p: ", data + i);
        }

        printf("%02x", value);
    }
    printf("\n");
}

void* handleSession(void* arg)
{
    MQTT_Session *session = (MQTT_Session *) arg;
    MQTT_ControlPacket *Packet = NULL;

    while (1)
    {
        Packet = mqttSrv->ACKForSession(mqttSrv, session);

        if (Packet->PacketType == PUBLISH)
            printf("topic:%s\nmessage:%s\n", Packet->VariableHeader->addr + 2, Packet->PayloadStart->addr);

        if (Packet->PacketType == DISCONNECT)
            break;
    }

}

void main(int argc, char** argv)
{
    mqttSrv = MQTT_ServerCreate("127.0.0.1", 1888);
    MQTT_Session *session = NULL;

    while (1)
    {
        session = mqttSrv->WaitForSession(mqttSrv);
        printf("%s: connect to %s at %d \n", __func__, inet_ntoa(session->Session->localAddr.sin_addr), ntohs(session->Session->localAddr.sin_port));
        taskCreate(handleSession, session);
    }

    pause();
}

