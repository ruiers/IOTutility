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

void* tcp_send(void* arg)
{
    int len = 0, i;

    while (1)
    {
        if ((targetTcp != NULL) && targetTcp->Connected)
        {
            targetTcp->Send(targetTcp, common_message, sizeof(common_message));
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

    targetUdp = udpClientCreate(5055);

    taskCreate(udp_srv, targetUdp);
    taskCreate(tcp_send, targetTcp);

    pause();
}
