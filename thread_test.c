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
void* thread(udpServer* uSvr)
{
    taskSetName("THREAD1");
    while (1)
    {

        semTake(ready, -1);
        log_dbg("recv from %s", uSvr->ip_str);
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

    while (cmd = getchar())
    {
        switch (cmd)
        {
        case 0x7a:
            uSvr->running = 0x7a;
            break;
        default:
            break;
        }
        log_ver("cmd = %x\n", cmd);
    }
    return;
}
