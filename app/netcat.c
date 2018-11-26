#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include "TcpClass.h"
#include "UdpClient.h"
#include "debug.h"

enum
{
    USE_TCP,
    USE_UDP
};

typedef struct _netcat_
{
    char remote_ip[16];
    int  remote_port;
    int  is_server;
    int  protocal;
    TcpClient* tcp_client;
    TcpListener* tcp_listener;
    UdpClient* udp_client;
} netcat;

int dataAvaliable(int fd)
{
    fd_set rfds;
    struct timeval timeout;
    int ret;

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(fd, &rfds);

    ret = select(fd + 1, &rfds, NULL, NULL, &timeout);

    if (ret == -1)
    {
        return ret;
    }
    else if (ret == 0)
    {
        return ret;
    }

    ret = 0x10;

    if (FD_ISSET(0, &rfds))
    {
        ret += 0x4000;
    }


    if (FD_ISSET(fd, &rfds))
    {
        ret += 0x8000;
    }

    return ret;
}

int main(int argc, char const *argv[])
{
    netcat instance;
    char recv_buf[1024];
    int  recv_len = 0;
    int i = 0, ret = 0;

    if (argc < 2)
        return 0;

    log_to_local_file("/dev/stdout");
    memset(&instance, 0x0, sizeof(netcat));

    for (i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--host"))
        {
            strcpy(instance.remote_ip, argv[++i]);
        }

        if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
        {
            instance.remote_port = atoi(argv[++i]);
        }

        if(!strcmp(argv[i], "-u"))
        {
            instance.protocal = USE_UDP;
        }

        if(!strcmp(argv[i], "-l"))
        {
            instance.is_server = 1;
        }
    }

    log_ver("netcat start remote ip %s port %d, use protocal %s %s", instance.remote_ip, instance.remote_port,
            instance.protocal == USE_TCP ? "tcp" : "udp", instance.is_server == 1 ? "server" : "client");

    if (instance.protocal == USE_TCP)
    {
        if (instance.is_server)
            instance.tcp_listener = tcpListenerCreate(instance.remote_ip, instance.remote_port);
        else
            instance.tcp_client = tcpClientCreate(instance.remote_ip, instance.remote_port);
    }
    else
    {
        if (instance.is_server)
            instance.udp_client = udpClientCreate(instance.remote_port);
        else
        {
            instance.udp_client = udpClientCreate(instance.remote_port - 1);
            instance.udp_client->Connect(instance.udp_client, instance.remote_ip, instance.remote_port);
            log_dbg("udp %d", instance.udp_client->Active);
        }
    }

    log_dbg("tcp %x %x udp %x", instance.tcp_listener, instance.tcp_client, instance.udp_client);

    while (1)
    {
        TcpClient *client;

        if (instance.is_server)
        {
            if (instance.protocal == USE_TCP)
            {
                instance.tcp_listener->Start(instance.tcp_listener);
                client = instance.tcp_listener->AcceptTcpClient(instance.tcp_listener);

                while (1)
                {
                    ret = dataAvaliable(client->Client);

                    if (ret & 0x4000)
                    {
                        recv_len = read(0, recv_buf, sizeof(recv_buf));
                        client->Send(client, recv_buf, recv_len);
                    }

                    if (ret & 0x8000)
                    {
                        recv_len = client->Receive(client, recv_buf, sizeof(recv_buf));

                        if (recv_len <= 0)
                            break;

                        recv_buf[recv_len] = '\0';
                        log_ver("%s", recv_buf);
                    }
                }
            }
            else
            {
                ret = dataAvaliable(instance.udp_client->Client);

                if (ret & 0x4000)
                {
                    recv_len = read(0, recv_buf, sizeof(recv_buf));
                    instance.udp_client->Send(instance.udp_client, recv_buf, recv_len);
                }

                if (ret & 0x8000)
                {
                    recv_len = instance.udp_client->Receive(instance.udp_client, recv_buf, sizeof(recv_buf));

                    if (recv_len <= 0)
                        break;

                    recv_buf[recv_len] = '\0';
                    log_ver("%s", recv_buf);
                }
            }
        }
        else
        {

            if (instance.protocal == USE_TCP)
            {
                ret = dataAvaliable(instance.tcp_client->Client);

                if (ret & 0x4000)
                {
                    recv_len = read(0, recv_buf, sizeof(recv_buf));
                    instance.tcp_client->Send(instance.tcp_client, recv_buf, recv_len);
                }

                if (ret & 0x8000)
                {
                    recv_len = instance.tcp_client->Receive(instance.tcp_client, recv_buf, sizeof(recv_buf));

                    if (recv_len <= 0)
                        break;

                    recv_buf[recv_len] = '\0';
                    log_ver("%s", recv_buf);
                }
            }
            else
            {
                ret = dataAvaliable(instance.udp_client->Client);

                if (ret & 0x4000)
                {
                    recv_len = read(0, recv_buf, sizeof(recv_buf));
                    instance.udp_client->Send(instance.udp_client, recv_buf, recv_len);
                }

                if (ret & 0x8000)
                {
                    instance.udp_client->Receive(instance.udp_client, recv_buf, sizeof(recv_buf));
                    log_ver("%s", recv_buf);
                }

            }

        }
    }

    return 0;
}
