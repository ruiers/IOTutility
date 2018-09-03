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

#if 0
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
            array = stream->AddByteArray(stream, cmd, 50);
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

#if 0

struct mqtt_connect
{
    char head_flag;
    char msg_len;
    short protocol_name_len;
    char protocol_name[6];
    char mqtt_version;
    char connect_flag;
    short keep_alive;
    short client_id_len;
    char client_id[23];
};

struct mqtt_publish
{
    char head_flag;
    char msg_len;
    short topic_len;
    char  topic[11];
    char  message[5];
};


int main()
{
    TcpClient* tcpClient = tcpClientCreate("10.56.56.236", 2022);
    char recvBuf[1500];
    int  recvLen = 0;
    //recvLen = tcpClient->Receive(tcpClient, recvBuf);
    log_ver("%d:%s", recvLen, recvBuf);
    struct mqtt_connect con =
    {
        .head_flag = 0x10,
        .msg_len = 37,
        .protocol_name_len = htons(6),
        .mqtt_version = 3,
        .connect_flag = 0x02,
        .keep_alive = htons(60),
        .client_id_len = htons(23),
    };

    struct mqtt_publish pub =
    {
        .head_flag = 0x30,
        .msg_len = 18,
        .topic_len = htons(11),

    };

    strncpy(con.protocol_name, "MQIsdp", 6);
    strncpy(con.client_id, "mosqpub/395-DESKTOP-S49", 23);
    tcpClient->Send(tcpClient, &con, sizeof(con) - 1);
    //recvLen = tcpClient->Receive(tcpClient, recvBuf);
    log_ver("%d:%s", recvLen, recvBuf);
    getchar();
    strncpy(pub.topic, "/kemov/test", 11);
    strncpy(pub.message, "/kemov/hello", 5);
    tcpClient->Send(tcpClient, &pub, sizeof(pub));
    //recvLen = tcpClient->Receive(tcpClient, recvBuf);
    log_ver("%d:%s", recvLen, recvBuf);
    //printf("paused\n");
    pause();
}
#endif


void main()
{
    TcpClient* tcpClient = tcpClientCreate("198.41.30.241", 1883);
    MQTT_ControlPacket*  mqttConnect = MQTT_ControlPacketCreate(CONNECT);
    MQTT_ControlPacketGetPacketData(mqttConnect);
    tcpClient->Send(tcpClient, mqttConnect->PacketData, mqttConnect->PacketLength);

    MQTT_ControlPacket*  mqttPublish = MQTT_ControlPacketCreate(PUBLISH);
    MQTT_ControlPacketSetTopic(mqttPublish, "wuhan/test", 10);
    MQTT_ControlPacketSetMessage(mqttPublish, "12", 2);
    MQTT_ControlPacketGetPacketData(mqttPublish);
    tcpClient->Send(tcpClient, mqttPublish->PacketData, mqttPublish->PacketLength);

    MQTT_ControlPacket*  mqttDisconnect = MQTT_ControlPacketCreate(DISCONNECT);
    MQTT_ControlPacketGetPacketData(mqttDisconnect);
    tcpClient->Send(tcpClient, mqttDisconnect->PacketData, mqttDisconnect->PacketLength);
}
