#include "linux/connect.h"
#include "linux/thread.h"
#include <stdio.h>
int udpServerStart(udpServer* udpSvr)
{
    int  udp_svr_fd;
    char p_recv_buff[1500];
    int  current_cmd = 0;

    struct sockaddr_in listener_addr;
    struct sockaddr_in server_addr;

    if (udpSvr->running)
        return 0;

    int recv_len = 0;
#ifdef OS_VXWORKS
    listener_addr.sin_len    = (UINT8)sizeof(struct sockaddr_in);
#endif
    listener_addr.sin_family      = AF_INET;
    listener_addr.sin_port        = htons(udpSvr->port);
    listener_addr.sin_addr.s_addr = htonl(INADDR_ANY)/*inet_addr("192.168.215.158");*/;

    if ((udp_svr_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket create err!\n");
        udpSvr->running = 0;
        return -1;
    }

    if (bind(udp_svr_fd, (struct sockaddr *)(&listener_addr), sizeof(struct sockaddr)) == -1)
    {
        printf("connect socket bind err!\n");
    }

    while (1)
    {
        udpSvr->running = 1;
        socklen_t len = sizeof(server_addr);
        recv_len = recvfrom(udp_svr_fd, (caddr_t) p_recv_buff, sizeof(p_recv_buff), 0, (struct sockaddr *)&server_addr, (int *)&len);
        current_cmd =  *(p_recv_buff);
        stpcpy(udpSvr->ip_str, inet_ntoa(server_addr.sin_addr));

        if (current_cmd == 0x7a)
        {
            close(udp_svr_fd);
            udpSvr->running = 0;
            return 0;
        }
        else
        {
            if (udpSvr->call_back != NULL)
                udpSvr->call_back(p_recv_buff, recv_len);
        }
    }
}

int udpServerThreadStart(udpServer* udpSvr)
{
    taskCreate((void *)udpServerStart, udpSvr);
}