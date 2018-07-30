#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct udp_server
{
    char  ip_str[12];
    int   port;
    int   running;
    void* (*call_back) (char *p_data, int length);
} udpServer;

int udpServerStart(udpServer* udpSvr);
int udpServerThreadStart(udpServer* udpSvr);
int udpSendto(char *destIP, int destPort, char *data, int len);

typedef struct tcp_client
{
    char  ip_str[12];
    int   port;
    int   fd;
    int   connected;
    void* (*call_back) (char *p_data, int length);
} tcpClient;

int tcpThreadConnect(tcpClient* tcpClt);
int tcpSendToServer(tcpClient* tcpClt, char* p_send_buf, int n_recv_len);