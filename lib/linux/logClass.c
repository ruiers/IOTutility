#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <sys/stat.h>
#include "TcpClass.h"
#include "debug.h"

enum log_type
{
    LOCAL_CON,
    LOCAL_FILE,
    NET_UDP,
    NET_TCP
};

FILE *log_steam = NULL;
char log_type = LOCAL_CON;
TcpClient* log_net = NULL;

void log_out_to(int fd)
{
    log_steam = fdopen (dup(fd), "w");
    setlinebuf(log_steam);
}

void init_log_to_file(char* name)
{
    int file_fd = 0;

    file_fd = open(name,   O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if (file_fd > 2)
    {
        log_out_to(file_fd);
        log_type = LOCAL_FILE;
    }
    else
    {
        close(file_fd);
    }
}

void init_log_to_net(char* host, int port)
{
    log_net = tcpClientCreate(host, port);

    log_out_to(log_net->Client);
    log_type = NET_TCP;
}

int log_buf(const char *format, ...)
{
    va_list arg;
    int done;

    va_start (arg, format);

    if (log_type == LOCAL_CON)
        done = vfprintf (stdout, format, arg);
    else if (log_type == LOCAL_FILE)
    {
        if (log_steam != NULL)
            done = vfprintf (log_steam, format, arg);
    }
    else if (log_type == NET_TCP)
    {
        if (log_net)
        {
            done = vfprintf (log_steam, format, arg);
        }
    }

    va_end (arg);

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
