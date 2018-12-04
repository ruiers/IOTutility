#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "TcpClass.h"
#include "UdpClient.h"
#include "debug.h"

static log_dev* log_devs[MAX_LOG_DEV_NUM];
static int      log_num = 0;

void log_to_none()
{
    int index = 0;

    for (index = 0; index < log_num; index++)
    {
        if (log_devs[index]->log_type == LOCAL_FILE)
        {
            close(log_devs[index]->log_fd);
        }
        else if (log_devs[index]->log_type == ETHER_NTCP)
        {
            TcpClient* tcpClient = (TcpClient *) log_devs[index]->netClient;
            tcpClient->Disconnect(tcpClient);
        }

        log_devs[index]->log_fd    = 0;
        log_devs[index]->log_steam = NULL;
        log_devs[index]->log_type  = LOCAL_NONE;
        log_devs[index]->netClient = NULL;
        free(log_devs[index]);
        log_devs[index] = NULL;
    }

    log_num = 0;
}

void log_to_local_file(char* name)
{
    int file_fd = open(name,   O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    FILE* filp = NULL;
    int index = log_num;

    if (index >= MAX_LOG_DEV_NUM)
    {
        return;
    }

    if (file_fd > 2)
    {
        filp = fdopen(dup(file_fd), "w");

        if (filp == NULL)
        {
            close(file_fd);
            return;
        }
        setlinebuf(filp);
        log_devs[index] = (log_dev *)malloc(sizeof(log_dev));
        log_devs[index]->log_steam = filp;
        log_devs[index]->log_fd    = file_fd;
        log_devs[index]->log_type  = LOCAL_FILE;
        log_num++;
    }
    else
    {
        close(file_fd);
    }
}

void log_to_ether_ntcp(char* host, int port)
{
    TcpClient* tcpClient = tcpClientCreate(host, port);
    FILE* filp = NULL;
    int index = log_num;

    if (index >= MAX_LOG_DEV_NUM)
    {
        return;
    }

    if (tcpClient == NULL)
        return;

    if (tcpClient->Connected == 0)
        return;

    filp = fdopen(dup(tcpClient->Client), "w");
    setlinebuf(filp);
    log_devs[index] = (log_dev *)malloc(sizeof(log_dev));
    log_devs[index]->netClient = tcpClient;
    log_devs[index]->log_steam = filp;
    log_devs[index]->log_fd   = tcpClient->Client;
    log_devs[index]->log_type = ETHER_NTCP;
    log_num++;
}

void log_to_ether_nudp(char* host, int port)
{
    UdpClient* udpClient = udpClientCreate(1887);
    FILE* filp = NULL;
    int index = log_num;

    if (index >= MAX_LOG_DEV_NUM)
    {
        return;
    }

    if (udpClient == NULL)
        return;
    else
        udpClient->Connect(udpClient, host, port);

    filp = fdopen(dup(udpClient->Client), "w");
    setlinebuf(filp);
    log_devs[index] = (log_dev *)malloc(sizeof(log_dev));
    log_devs[index]->netClient = udpClient;
    log_devs[index]->log_steam = filp;
    log_devs[index]->log_fd   = udpClient->Client;
    log_devs[index]->log_type = ETHER_NUDP;
    log_num++;
}

int log_buf(const char *format, ...)
{
    va_list arg;
    int done, index;

    for (index = 0; index < log_num; index++)
    {
        va_start (arg, format);

        if (log_devs[index]->log_steam)
            done = vfprintf (log_devs[index]->log_steam, format, arg);

        va_end (arg);
    }

    return done;
}

void hexdump(char* data, int len)
{
    int i = 0, value = 0;
    for (i = 0; i < len; i++)
    {
        value = *((unsigned char *) data + i);
        if ((i) % 2 == 0)
        {
            log_buf(" ");
        }
        if ((i) % 16 == 0)
        {
            log_buf("\n");
            log_buf("%p: ", data + i);
        }
        log_buf("%02x", value);
    }
    log_buf("\n");
}
