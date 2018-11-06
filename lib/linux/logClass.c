#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "TcpClass.h"
#include "debug.h"

enum log_type
{
    LOCAL_CON  = 1 << 0,
    LOCAL_FILE = 1 << 1,
    NET_UDP    = 1 << 2,
    NET_TCP    = 1 << 3
};

FILE *log_steam_file = NULL;
FILE *log_steam_ntcp = NULL;
char log_type = LOCAL_CON;
TcpClient* log_net = NULL;

int init_log_to_file(char* name)
{
    int file_fd = 0;

    file_fd = open(name,   O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if (file_fd > 2)
    {
        log_steam_file = fdopen(dup(file_fd), "w");
        setlinebuf(log_steam_file);
        log_type |= LOCAL_FILE;
    }
    else
    {
        close(file_fd);
        file_fd = 0;
    }

    return file_fd;
}

void free_log_to_file(int fd)
{
    if (fd > 0)
        close(fd);

    log_type &= ~LOCAL_FILE;
    log_steam_file = NULL;
}

void init_log_to_net(char* host, int port)
{
    log_net = tcpClientCreate(host, port);

    if (log_net == NULL)
        return;

    if (log_net->Connected == 0)
        return;

    log_steam_ntcp = fdopen(dup(log_net->Client), "w");
    setlinebuf(log_steam_ntcp);

    log_type |= NET_TCP;
}

void free_log_to_net()
{
    if ((log_net) && (log_net->Client > 0))
        log_net->Disconnect(log_net);

    log_type &= ~NET_TCP;
    log_steam_ntcp = NULL;
    free(log_net);
    log_net = NULL;
}

int log_buf(const char *format, ...)
{
    va_list arg, *logarg;
    int done;

    logarg = (va_list *) malloc(sizeof(va_list));

    va_start (arg, format);

    if (log_type & LOCAL_CON)
    {
        memcpy(logarg, &arg, sizeof(arg));
        done = vfprintf (stdout, format, *logarg);
    }

    if (log_type & LOCAL_FILE)
    {
        if (log_steam_file != NULL)
        {
            memcpy(logarg, &arg, sizeof(arg));
            done = vfprintf (log_steam_file, format, *logarg);
        }
    }

    if (log_type & NET_TCP)
    {
        if (log_net && log_steam_ntcp)
        {
            memcpy(logarg, &arg, sizeof(arg));
            done = vfprintf (log_steam_ntcp, format, *logarg);
        }
    }

    va_end (arg);

    free(logarg);

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
