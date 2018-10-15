#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

    this->remoteAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ipStr, &this->remoteAddr.sin_addr);
    this->remoteAddr.sin_port = htons(portNum);

    if (0 == this->Client)
    {
        this->Client = socket(AF_INET, SOCK_STREAM, 0);
    }

    if (0 == connect(this->Client, (struct sockaddr *)&this->remoteAddr, sizeof(this->remoteAddr)))
        this->Connected = 1;
    else
    {
        this->Connected = 0;
        printf("%s: connect to %s at %d failed\n", __func__, inet_ntoa(this->remoteAddr.sin_addr), ntohs(this->remoteAddr.sin_port));
        perror("");
        close(this->Client);
    }
}

void tcpClientDisconnect(TcpClient* this)
{
    if ((this != NULL) && (this->Client != 0))
    {
        shutdown(this->Client, SHUT_RDWR);
        close(this->Client);
        this->Client = 0;
        this->Connected = 0;
    }
}

int tcpClientSend(TcpClient* this, char* data_addr, int data_len)
{
    int length = 0;

    if (this->Connected)
    {
        length = write(this->Client, data_addr, data_len);

        if (length == data_len)
            return length;
    }

    return -1;
}

int tcpClientReceive(TcpClient* this, char* data_addr, int data_len)
{
    int length = 0;

    if (this->Connected)
    {
        length = read(this->Client, data_addr, data_len);

        if (length <= data_len)
            return length;
    }

    return -1;
}

TcpClient* tcpClientCreate(char* serverIP, int serverPort)
{
    TcpClient* client = calloc(1, sizeof(TcpClient));

    client->remoteAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP, &client->remoteAddr.sin_addr);
    client->remoteAddr.sin_port = htons(serverPort);

    if ((client->Client = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket create err!\n");
        client->Active = 0;
        return NULL;
    }

    tcpClientConnect(client, serverIP, serverPort);

    client->Send    = tcpClientSend;
    client->Receive = tcpClientReceive;
    client->Connect = tcpClientConnect;
    client->Disconnect = tcpClientDisconnect;

    return client;
}

void TcpListenerStart(struct _TcpListener_ *this)
{
    if(listen(this->ListenSock, 10) < 0)
    {
        printf("listen failed\n");
        close(this->ListenSock);
    }
}

void TcpListenerStop(struct _TcpListener_ *this)
{

}

TcpClient* TcpListenerAcceptTcpClient(struct _TcpListener_ *this)
{
    TcpClient* tcpCon = (TcpClient *) calloc(1, sizeof(TcpClient));
    socklen_t len = sizeof(struct sockaddr_in);

    tcpCon->Client = accept(this->ListenSock, (struct sockaddr*)&tcpCon->localAddr, &len);

    if(tcpCon->Client < 0)
    {
        printf("accept failed\n");
        return NULL;
    }

    tcpCon->Connected = 1;
    tcpCon->Send = tcpClientSend;
    tcpCon->Receive = tcpClientReceive;

    return tcpCon;
}

TcpListener* tcpListenerCreate(char* IPStr, int portNum)
{
    TcpListener* listener = calloc(1, sizeof(TcpListener));

    struct sockaddr_in SelfAddrIn;

    memset(&SelfAddrIn, 0x0, sizeof(SelfAddrIn));
    SelfAddrIn.sin_family = AF_INET;
    SelfAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
    SelfAddrIn.sin_port = htons(portNum);

    if ((listener->ListenSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket create err!\n");
        return NULL;
    }

    if (bind(listener->ListenSock, (struct sockaddr*)&SelfAddrIn, sizeof(struct sockaddr_in))<0)
    {
        printf("socket bind error\n");
        close(listener->ListenSock);
        return NULL;
    }

    listener->Start = TcpListenerStart;
    listener->Stop  = TcpListenerStop;
    listener->AcceptTcpClient = TcpListenerAcceptTcpClient;

    return listener;
}
