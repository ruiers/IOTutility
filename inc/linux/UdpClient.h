#ifndef __INC_LINUX_UDPCLIENT__
#define __INC_LINUX_UDPCLIENT__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

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
    int  (*Receive) (struct UdpClient_T* this, char* data_addr);
} UdpClient;

UdpClient* udpClientCreate(int portNum);

#endif
