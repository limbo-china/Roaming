#include "client.h"
#include "jsontostruct.h"

const int tr2KeepAlive = 10;
const int waitReconn = 5;
int g_sockfd = 0;

const char* test_json = "{\"RoamProvince\": \"31\",\"Region\": \"21512\",\"UserNumber\": \"8613899050320\",\"Time\": \"132817183113\",\"Action\": \"1\"}";


void* heartBeatDetect()
{
    alarmHandler();
    while (1)
        ;
    return NULL;
}
void alarmHandler()
{
    if (g_sockfd == 0) {
        signal(SIGALRM, alarmHandler);
        alarm(tr2KeepAlive);
        return;
    }

    sendHBMsg(g_sockfd); //发送心跳包进行检测

    signal(SIGALRM, alarmHandler); //重新定时
    alarm(tr2KeepAlive);
}
void sendHBMsg(int _sock)
{
    HB_MsgContent hbmsg = constructHBMsg();

    int n;
    if ((n= send(_sock, &hbmsg, sizeof(hbmsg),0)) < 0) {
        //
        /////reconnect
        printf("reconnecting to server ...\n");

        close(g_sockfd);
        int t = connectToServ();
        if (t != 0)
            g_sockfd = t;

        printf("reconnect succeed.\n");
    }
    printf("hb :%d\n",n );
}
void sendFRepMsg(int _sock)
{
    FRep_MsgContent frepmsg = constructFRepMsg();

    int n;
    if ((n = send(_sock, &frepmsg, sizeof(frepmsg),0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //
    }
    printf("frep: %d\n", n);
}
void sendRDataMsg(int _sock){
    RData_MsgContent *rdata = j2s(test_json);

    int n;
    if ((n = send(_sock, rdata, sizeof(RData_MsgContent),0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //
    }
    printf("rdata: %d\n", n);

}
int connectToServ()
{

    char server_ip[20] = { 0 };
    int server_port = 0;
    struct sockaddr_in servaddr;

    if (!getSrvCfg(server_ip, &server_port)) {
        printf("get server configuration failed.\n");
        return 0;
    }

    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    servaddr = initSockAddr(server_ip, server_port);

    printf("connecting to server ...\n");
    Connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("connect succeed.\n");

    return sockfd;
}
void* roamClient()
{
    int t = connectToServ();
    if (t != 0)
        g_sockfd = t;
    else
        return NULL;

    while (1) {
        sleep(1);
        sendRDataMsg(g_sockfd);
    }
    return NULL;
}




