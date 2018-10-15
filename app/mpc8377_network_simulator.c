#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "thread.h"
#include "TcpClass.h"
#include "UdpClient.h"
#include <arpa/inet.h>

char server_ip_string[] = "127.0.0.1";
int  server_tcp_port    = 5067;

char common_message[]   = "keepAlive";
char recved_data[512];

TcpClient *targetTcp = NULL;
UdpClient *targetUdp = NULL;

void dHex(char *data, int len)
{
    int i = 0, value = 0;
    for (i = 0; i < len; i++)
    {
        value = *((unsigned char *) data + i);
        printf("%02x ", value);
    }
    printf("\n");
}

void* tcp_send(void* arg)
{
    int len = 0, i;

    while (1)
    {
        if ((targetTcp != NULL) && targetTcp->Connected)
        {
            len = targetTcp->Send(targetTcp, common_message, sizeof(common_message));

            if (len < 0)
                targetTcp->Disconnect(targetTcp);
        }

        sleep(1);
    }
}

void* udp_srv(void* arg)
{
    UdpClient *target = (UdpClient *) arg;
    int len = 0, i;

    while (1)
    {
        len = target->Receive(target, recved_data, 512);

        if ('c' == *recved_data)
        {
            if (targetTcp == NULL)
                targetTcp = tcpClientCreate(inet_ntoa(target->destAddr.sin_addr), server_tcp_port);
            else
                targetTcp->Connect(targetTcp, inet_ntoa(target->destAddr.sin_addr), server_tcp_port);
        }

        if ('d' == *recved_data)
        {
            if (targetTcp != NULL)
                targetTcp->Disconnect(targetTcp);
        }

        memset(recved_data, 0x0, sizeof(recved_data));
    }
}

void main()
{
    int i = 0, len = 0, value = 0, data[10];
    targetUdp = udpClientCreate(5055);

    taskCreate(udp_srv, targetUdp);
    taskCreate(tcp_send, targetTcp);

    while (1)
    {
        if (targetTcp != NULL)
        {
            if (targetTcp->Connected == 0)
            {
                sleep(1);
                continue;
            }

            len = targetTcp->Receive(targetTcp, recved_data, 512);
            dHex(recved_data, len);

            for (i = 0; i< len/4; i++)
            {
                value = htonl(*((int *) recved_data + i) );
                printf("%x ", value);
            }

            printf("\n");
        }
    }
    pause();
}
