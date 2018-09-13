#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "linux/connect.h"
#include "linux/thread.h"

int udpServerStart(udpServer* udpSvr)
{
    int  udp_svr_fd;
    char p_recv_buff[1500];

    struct sockaddr_in listener_addr;
    struct sockaddr_in server_addr;

    if (udpSvr->running == 1)
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

        stpcpy(udpSvr->ip_str, inet_ntoa(server_addr.sin_addr));

        if (udpSvr->running == 0x7a)
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

int udpSendto(char *destIP, int destPort, char *data, int len)
{
    struct sockaddr_in destAddr;
    int 			   sockFd;
#ifdef OS_VXWORKS
    destAddr.sin_len         = (char*) sizeof(struct sockaddr_in);
#endif
    destAddr.sin_family      = AF_INET;
    destAddr.sin_port        = htons(destPort);
    destAddr.sin_addr.s_addr = inet_addr(destIP);;

    if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket create err!\n");
        close(sockFd);
        return -1;
    }

    if (sendto(sockFd, (caddr_t)data, len, 0, (struct sockaddr *)&destAddr, sizeof(destAddr)) == -1)
    {
        printf("udp sendto %s @ %d failed!\n", destIP, destPort);
        perror("");
        close(sockFd);
        return -1;
    }

    close(sockFd);
    return 0;
}

int recvFromServer(tcpClient* tcpClt, char* p_recv_buf, int n_recv_len)
{
    return read(tcpClt->fd, p_recv_buf, n_recv_len);
}

int tcpSendToServer(tcpClient* tcpClt, char* p_send_buf, int n_recv_len)
{
    if (tcpClt->connected)
        return write(tcpClt->fd, p_send_buf, n_recv_len);
    else
        return 0;
}

void tcpSendFileToServer(tcpClient* tcpClt, char* filePath)
{
    struct stat s_buf;

    stat(filePath, &s_buf);

    if(S_ISREG(s_buf.st_mode))
    {
        int filefd = open( filePath, O_RDONLY);

        if (tcpClt->connected)
            sendfile(tcpClt->fd, filefd, NULL, s_buf.st_size );

        close(filefd);
    }
}

void tcpConnectToServer(tcpClient* tcpClt)
{
    struct sockaddr_in serverAddr;
    tcpClt->fd = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, tcpClt->ip_str, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(tcpClt->port);

    if (0 == connect(tcpClt->fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
        tcpClt->connected = 1;
    else
    {
        tcpClt->connected = 0;
        printf("%s: connect to %s at %d failed\n", __func__, tcpClt->ip_str, tcpClt->port);
        perror("");
        close(tcpClt->fd);
    }

}

void tcpPullFromServer(tcpClient* tcpClt)
{
    char p_recv_buff[1024];
    int  n_recv_len = 0;

    while (tcpClt->connected > 0)
    {
        n_recv_len = recvFromServer(tcpClt, p_recv_buff, 1024);

        if (n_recv_len > 0 && tcpClt->call_back != NULL)
            tcpClt->call_back(p_recv_buff, n_recv_len);
        else
        {
            tcpClt->connected = 0;
            close(tcpClt->fd);
            break;
        }
    }
}

int tcpThreadConnect(tcpClient* tcpClt)
{
    tcpConnectToServer(tcpClt);
    taskCreate((void *)tcpPullFromServer, tcpClt);
}

int tcpDisconnect(tcpClient* tcpClt)
{
    close(tcpClt->fd);
    tcpClt->connected = 0;
    return 0;
}

