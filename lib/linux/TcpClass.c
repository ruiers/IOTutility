#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "TcpClass.h"
#ifdef OS_VXWORKS
#include <sockLib.h>
#include <inetLib.h>
#endif

void tcpClientConnect(TcpClient* this, char* ipStr, int portNum)
{
    if (this->Connected)
        return;

    this->servAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ipStr, &this->servAddr.sin_addr);
    this->servAddr.sin_port = htons(portNum);

    if (0 == connect(this->Client, (struct sockaddr *)&this->servAddr, sizeof(this->servAddr)))
        this->Connected = 1;
    else
    {
        this->Connected = 0;
        printf("%s: connect to %s at %d failed\n", __func__, inet_ntoa(this->servAddr.sin_addr), ntohs(this->servAddr.sin_port));
        perror("");
        close(this->Client);
    }
}

void tcpClientDisconnect(TcpClient* this)
{
    close(this->Client);
    this->Connected = 0;
}

int tcpClientSend(TcpClient* this, char* data_addr, int data_len)
{
    if (this->Connected)
    {
        return write(this->Client, data_addr, data_len);
    }
    else
        return 0;
}

int tcpClientReceive(TcpClient* this, char* data_addr, int data_len)
{
    return read(this->Client, data_addr, data_len);
}

TcpClient* tcpClientCreate(char* serverIP, int serverPort)
{
    TcpClient* client = calloc(1, sizeof(TcpClient));

    client->servAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP, &client->servAddr.sin_addr);
    client->servAddr.sin_port = htons(serverPort);

    if ((client->Client = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket create err!\n");
        client->Active = 0;
        return NULL;
    }

    if (0 == connect(client->Client, (struct sockaddr *)&client->servAddr, sizeof(client->servAddr)))
        client->Connected = 1;
    else
    {
        client->Connected = 0;
        printf("%s: connect to %s at %d failed\n", __func__, inet_ntoa(client->servAddr.sin_addr),client->servAddr.sin_port);
        perror("");
        close(client->Client);
    }

    client->Send    = tcpClientSend;
    client->Receive = tcpClientReceive;
    client->Connect = tcpClientConnect;
    client->Disconnect = tcpClientDisconnect;

    return client;
}
