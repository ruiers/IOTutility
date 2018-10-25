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

MQTT_Server *mqttSrv = NULL;
char buf[512] = { 0 };

void* handleSession(void* arg)
{
    MQTT_Session *session = (MQTT_Session *) arg;
    MQTT_ControlPacket *Packet = NULL;

    while (1)
    {
        Packet = mqttSrv->ACKForSession(mqttSrv, session);

        if (Packet->PacketType == PUBLISH)
        {
            printf("topic:%s\nmessage:%s\n", Packet->VariableHeader->addr + 2, Packet->PayloadStart->addr);
            log_hex(Packet->ControlPacket->Memory, Packet->ControlPacket->Length);
            session->Session->Send(session->Session, Packet->ControlPacket->Memory, Packet->ControlPacket->Length);
        }

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

