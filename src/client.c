#include "client.h"

const int tr2KeepAlive = 3;

int g_sockfd = 0;

void *heartBeatDetect()
{
    alarmHandler();
    while(1);
    return NULL;
}
void alarmHandler()
{
    if (g_sockfd == 0)
    {
        signal(SIGALRM, alarmHandler);
        alarm(tr2KeepAlive);
        return;
    }

    sendHBMsg(g_sockfd);  //发送心跳包进行检测

    signal(SIGALRM, alarmHandler);  //重新定时
    alarm(tr2KeepAlive);
}
void sendHBMsg(int _sock)
{
    HB_MsgContent hbmsg = constructHBMsg();

    if (sendn(_sock, &hbmsg, sizeof(hbmsg)) < 0)
    {
        //

        /////连接断开,重新连接
    }
}
void sendFRepMsg(int _sock)
{
    FRep_MsgContent frepmsg = constructFRepMsg();

    sendn(_sock, &frepmsg, sizeof(frepmsg));
}
void *roamClient()
{
    char server_ip[20] =
    {
        0
    };
    int server_port = 0;
    struct sockaddr_in servaddr;

    if (!getSrvCfg(server_ip, &server_port))
    {
        printf("get server configuration failed.\n");
        return NULL;
    }

    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    g_sockfd = sockfd;
    servaddr = initSockAddr(server_ip, server_port);

    printf("connecting to server ...\n");
    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    while(1)
    {
        sleep(1);
        sendFRepMsg(sockfd);
    }
    return NULL;
}




