#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "inc/base.h"


sem_t_id ready;

void* udpDataHandle(char* data, int len)
{
    log_dbg("%d:%s\n", len, data);
    semGive(ready);
}

void* tcpDataHandle(char* data, int len)
{
    log_dbg("%d:%s\n", len, data);
}

void* thread(udpServer* uSvr)
{
    taskSetName("THREAD1");
    while (1)
    {
        semTake(ready, -1);
        log_dbg("recv from %s", uSvr->ip_str);
        udpSendto(uSvr->ip_str, 5056, "ack", 3);
    }
}
void main()
{
    char cmd = 0x0;
    taskSetName("MainThread");
    ready = semCreate(0);

    log_err("test start\n");

    udpServer* uSvr = malloc(sizeof(udpServer));
    uSvr->port = 5055;
    uSvr->call_back = udpDataHandle;
    udpServerThreadStart(uSvr);
    taskCreate((void *) thread, uSvr);

    tcpClient* tcpClt = malloc(sizeof(tcpClient));
    strcpy(tcpClt->ip_str, "10.56.56.236");
    tcpClt->port = 5057;
    tcpClt->call_back = tcpDataHandle;
    tcpThreadConnect(tcpClt);
    log_dbg("tcp connect %d", tcpClt->connected);

    while (cmd = getchar())
    {
        if (cmd == 'Z')
            break;
        switch (cmd)
        {
        case 0x7a:
            uSvr->running = 0x7a;
            break;
        case 'l':
            tcpSendFileToServer(tcpClt, "./lock");
        default:
            break;
        }
        log_ver("cmd = %x\n", cmd);
        tcpSendToServer(tcpClt, "cmd", 3);
    }
    return;
}
