#ifndef __INC_LINUX_UDPCLIENT__
#define __INC_LINUX_UDPCLIENT__

#include <netinet/in.h>

typedef struct UdpClient_T
{
    int   Active;
    int   Available;
    int   Client;
    struct sockaddr_in destAddr;
    struct sockaddr_in servAddr;

    void (*Connect) (struct UdpClient_T* this, char* ipStr, int portNum);
    void (*Send)    (struct UdpClient_T* this, char* data_addr, int data_len);
    void (*Sendto)  (struct UdpClient_T* this, char* data_addr, int data_len, char* ipStr, int portNum);
    int  (*Receive) (struct UdpClient_T* this, char* data_addr, int data_len);
} UdpClient;

UdpClient* udpClientCreate(int portNum);

#endif
