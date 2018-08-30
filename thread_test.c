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

#if 0
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

    CacheList clh = MemoryCacheCreate(100,10);
    int id = 0;
    getchar();
    while (cmd = getchar())
    {
        MemoryCache* mc_insert, * mc_first ;

        if (cmd == 'Z')
            break;
        switch (cmd)
        {
        case 0x7a:
            uSvr->running = 0x7a;
            break;
        case 'l':
            tcpSendFileToServer(tcpClt, "./lock");
            break;
        case 'a':
            if (MemoryCacheIsFull(clh))
                MemoryCacheFree(clh, MemoryCacheGet(clh));

            mc_insert = MemoryCacheAlloc(clh);

            if (mc_insert != NULL)
            {
                mc_insert->cache_size = id++;
                *(mc_insert->cache_addr) = 'a' + id;
            }
            break;
        case 'd':
            MemoryCacheFree(clh, MemoryCacheGet(clh));
            break;
        case 'r':
            MemoryCacheReset(clh);
            id = 0;
            break;
        default:
            break;
        }
        log_ver("cmd = %x\n", cmd);
        tcpSendToServer(tcpClt, "cmd", 3);

        mc_first = MemoryCacheGet(clh);
        while(mc_first != NULL)
        {
            log_dbg("cache %d, %c, %p", mc_first->cache_size, *(mc_first->cache_addr), mc_first->cache_addr);
            mc_first = MemoryCacheNext(mc_first);
        }
    }
    return;
}
#endif

#if 0
void main()
{
    char cmd[50];
    taskSetName("MainThread");
    ready = semCreate(0);

    log_err("test start\n");

    MemoryStream stream = MemoryStreamCreate();

    UdpClient* udpClient = udpClientCreate(2022);

    udpClient->Connect(udpClient, "10.56.56.236", 2036);
    while (1)
    {
        MemoryByteArray* array;
        scanf("%s", cmd);
        if (cmd[0] == 'Z')
            break;
        switch (cmd[0])
        {
        case 'a':
            array = stream->AddByteArray(stream, 50);
            memcpy(array->addr, cmd, 50);
            udpClient->Send(udpClient, cmd, strlen(cmd));
            break;
        case 'd':
            stream->DeleteByteArray(stream, stream->GetByteArray(stream));
            udpClient->Sendto(udpClient, cmd, strlen(cmd), "127.0.0.1", 2035);
            break;
        case 'l':
            array = stream->GetByteArray(stream);
            while(array != NULL)
            {
                log_ver("history %s", array->addr);
                array = stream->NextByteArray(array);
            }
            udpClient->Receive(udpClient, cmd);
            break;
        default:
            break;
        }
        log_ver("cmd = %s\n", cmd);
    }
    return;
}
#endif

#if 1
void main()
{
    char cmd[50];
    taskSetName("MainThread");
    ready = semCreate(0);

    log_err("test start\n");

    MemoryStream stream = MemoryStreamCreate();

    TcpClient* tcpClient = tcpClientCreate("10.56.56.236", 2022);


    while (1)
    {
        MemoryByteArray* array;
        scanf("%s", cmd);
        if (cmd[0] == 'Z')
            break;
        switch (cmd[0])
        {
        case 'a':
            array = stream->AddByteArray(stream, 50);
            memcpy(array->addr, cmd, 50);
            tcpClient->Send(tcpClient, cmd, strlen(cmd));
            break;
        case 'd':
            stream->DeleteByteArray(stream, stream->GetByteArray(stream));
            tcpClient->Disconnect(tcpClient);
            break;
        case 'l':
            array = stream->GetByteArray(stream);
            while(array != NULL)
            {
                log_ver("history %s", array->addr);
                array = stream->NextByteArray(array);
            }
            tcpClient->Receive(tcpClient, cmd);
            break;
        default:
            break;
        }
        log_ver("cmd = %s\n", cmd);
    }
    return;
}
#endif

