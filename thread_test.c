#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "inc/base.h"


sem_t_id ready;

void* udpDataHandle(char* data, int len)
{
    log_dbg("%d:%s\n", len, data);
}
void* thread()
{
    taskSetName("THREAD1");
    while (1)
    {

        semTake(ready, -1);
        log_dbg("print");
    }
}
void main()
{
    taskSetName("MainThread");
    ready = semCreate(0);

    log_err("test start\n");

    taskCreate(thread, NULL);

    udpServer* uSvr = malloc(sizeof(udpServer));
    uSvr->port = 5055;
    uSvr->call_back = udpDataHandle;
    udpServerThreadStart(uSvr);

    while (1)
    {
        log_dbg("print");
        semGive(ready);

        sleep(5);
        log_err("restart");
    }
    return;
}
