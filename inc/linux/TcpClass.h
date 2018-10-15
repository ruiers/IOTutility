#ifndef __INC_LINUX_TCPCLASS__
#define __INC_LINUX_TCPCLASS__

#include <netinet/in.h>

typedef struct _TcpClient_
{
    int   Active;
    int   Available;
    int   Client;
    int   Connected;
    struct sockaddr_in remoteAddr;
    struct sockaddr_in localAddr;

    void (*Connect) (struct _TcpClient_* this, char* ipStr, int portNum);
    void (*Disconnect) (struct _TcpClient_* this);
    int  (*Send)    (struct _TcpClient_* this, char* data_addr, int data_len);
    int  (*Receive) (struct _TcpClient_* this, char* data_addr, int data_len);
} TcpClient;

TcpClient* tcpClientCreate(char* serverIP, int serverPort);

typedef struct _TcpListener_
{
    int ListenSock;
    void (*Start) (struct _TcpListener_ *this);
    void (*Stop) (struct _TcpListener_  *this);
    TcpClient* (*AcceptTcpClient) (struct _TcpListener_ *this);

} TcpListener;

TcpListener* tcpListenerCreate(char* IPStr, int portNum);
#endif
