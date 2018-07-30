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