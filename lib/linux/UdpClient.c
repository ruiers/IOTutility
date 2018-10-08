#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "UdpClient.h"
#ifdef OS_VXWORKS
#include <sockLib.h>
#include <inetLib.h>
#endif

void udpClientConnect(UdpClient* this, char* ipStr, int portNum)
{
    if (this->Active)
    {
        close(this->Client);
        memset(&this->destAddr, 0x0, sizeof(struct sockaddr_in));
    }

    this->destAddr.sin_family      = AF_INET;
    this->destAddr.sin_port        = htons(portNum);
    this->destAddr.sin_addr.s_addr = inet_addr(ipStr);

    if ((this->Client = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket create err!\n");
        close(this->Client);
        return ;
    }

    this->Active = 1;
}

void udpClientSend(UdpClient* this, char* data_addr, int data_len)
{
    if (this->Active < 1)
        return;

    if (sendto(this->Client, (caddr_t)data_addr, data_len, 0, (struct sockaddr *)&this->destAddr, sizeof(this->destAddr)) == -1)
    {
        printf("udp sendto %s @ %d failed!\n", inet_ntoa(this->destAddr.sin_addr), this->destAddr.sin_port);
        perror("");
        close(this->Client);
        return;
    }
}

void udpClientSendto(UdpClient* this, char* data_addr, int data_len, char* ipStr, int portNum)
{
    struct sockaddr_in addr_dest;

    addr_dest.sin_family      = AF_INET;
    addr_dest.sin_port        = htons(portNum);
    addr_dest.sin_addr.s_addr = inet_addr(ipStr);

    if (sendto(this->Client, (caddr_t)data_addr, data_len, 0, (struct sockaddr *)&addr_dest, sizeof(addr_dest)) == -1)
    {
        printf("udp sendto %s @ %d failed!\n", inet_ntoa(addr_dest.sin_addr), addr_dest.sin_port);
        perror("");
        close(this->Client);
        return;
    }
}

int udpClientReceive(UdpClient* this, char* data_addr, int data_len)
{
    socklen_t len = sizeof(this->servAddr);
    int ret = 0;

    ret =  recvfrom(this->Client, (caddr_t) data_addr, data_len, 0, (struct sockaddr *)&this->servAddr, (int *)&len);

    if (ret > 0)
        this->Active = 1;

    return ret;
}

UdpClient* udpClientCreate(int portNum)
{
    UdpClient* client = calloc(1, sizeof(UdpClient));

    client->servAddr.sin_family      = AF_INET;
    client->servAddr.sin_port        = htons(portNum);
    client->servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((client->Client = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket create err!\n");
        client->Active = 0;
        return NULL;
    }

    if (bind(client->Client, (struct sockaddr *)(&client->servAddr), sizeof(struct sockaddr)) == -1)
    {
        printf("connect socket bind err!\n");
        return NULL;
    }

    client->Connect = udpClientConnect;
    client->Send    = udpClientSend;
    client->Sendto  = udpClientSendto;
    client->Receive = udpClientReceive;

    return client;
}
