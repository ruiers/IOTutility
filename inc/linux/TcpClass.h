#ifndef __INC_LINUX_TCPCLASS__
#define __INC_LINUX_TCPCLASS__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

typedef struct TcpClient_T
{
    int   Active;
    int   Available;
    int   Client;
    int   Connected;
    struct sockaddr_in servAddr;

    void (*Connect) (struct TcpClient_T* this, char* ipStr, int portNum);
    void (*Disconnect) (struct TcpClient_T* this);
    int  (*Send)    (struct TcpClient_T* this, char* data_addr, int data_len);
    int  (*Receive) (struct TcpClient_T* this, char* data_addr, int data_len);
} TcpClient;

TcpClient* tcpClientCreate(char* serverIP, int serverPort);

#endif
