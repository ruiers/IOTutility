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

void main(int argc, char** argv)
{
    mqttSrv = MQTT_ServerCreate("127.0.0.1", 1888);

    int len;
    MQTT_Session *c = mqttSrv->WaitForSession(mqttSrv);

    printf("%s: connect to %s at %d \n", __func__, inet_ntoa(c->Session->localAddr.sin_addr), ntohs(c->Session->localAddr.sin_port));

    while (1)
    {
        mqttSrv->ACKForSession(mqttSrv, c);
    }

    pause();
}

