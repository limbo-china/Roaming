#include "client.h"

const int tr2KeepAlive = 3;
const int waitReconn = 5;
int g_sockfd = 0;
pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
hashtable_t* rdtable;

int datanum = 0;

void* requestDetect()
{
    int maxfdp1;
    fd_set rset, wset; // read set and write set
    int n;

    FD_ZERO(&rset);
    FD_ZERO(&wset);

    for (;;) {

        FD_SET(g_sockfd, &rset);
        //FD_SET(g_sockfd, &wset);
        maxfdp1 = g_sockfd + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(g_sockfd, &rset)) {
            FReq_MsgContent* freqmsg = (FReq_MsgContent*)malloc(sizeof(FReq_MsgContent));
            if ((n = recv(g_sockfd, freqmsg, sizeof(FReq_MsgContent), 0)) == 3) {
                if (freqmsg->msg_length == 3 && freqmsg->msg_type == 1) {
                    printf("received request message\n");
                    //sendFRepMsg(g_sockfd);
                    pthread_mutex_lock(&send_mutex);
                    sendAllRData(rdtable);
                    pthread_mutex_unlock(&send_mutex);
                    //sendFDataFinMsg(g_sockfd);
                }
            }
        }
    }
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
    if ((n = send(_sock, &hbmsg, sizeof(hbmsg), 0)) < 0) {
        //
        /////reconnect
        printf("reconnecting to server ...\n");

        close(g_sockfd);
        int t = connectToServ();
        //if (t != 0)
            g_sockfd = t;

        printf("reconnect succeed.\n");
    }
    printf("hb :%d\n", n);
}
void sendFRepMsg(int _sock)
{
    FRep_MsgContent frepmsg = constructFRepMsg();

    int n;
    if ((n = send(_sock, &frepmsg, sizeof(frepmsg), 0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //
    }
    printf("frep: %d\n", n);
}
void sendFDataFinMsg(int _sock)
{
    FData_FinMsgContent fdatafinmsg = constructFDataFinMsg();

    int n;
    if ((n = send(_sock, &fdatafinmsg, sizeof(fdatafinmsg), 0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //
    }
    printf("fdatafin: %d\n", n);
}
void sendRDataMsg(RData_MsgContent* rdata, int _sock)
{
    int n;
    datanum++;
    int i;
    printf("%d: ", datanum);
    printf("%u", rdata->length);
    printf("%u", rdata->type);
    printf("%u", rdata->roamprovince);
    printf("%u", ntohs(rdata->region));
    for (i = 0; i < 12; i++)
        printf("%c", *((u_char*)rdata->usernumber + i));
    printf("%u", ntohl(rdata->time));
    printf("%d", rdata->action);
    printf("\n");

    char _datamsg[RDATAMSG_LENGTH + 1];
    _datamsg[0] = rdata->length;
    _datamsg[1] = rdata->type;
    _datamsg[2] = rdata->roamprovince;
    strncpy(_datamsg + 3, (char*)&rdata->region, 2);
    strncpy(_datamsg + 5, rdata->usernumber, 12);
    strncpy(_datamsg + 17, (char*)&rdata->time, 4);
    _datamsg[21] = rdata->action;
    n = 0;
    if ((n = send(_sock, _datamsg, RDATAMSG_LENGTH, 0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //

        // printf("reconnecting to server ...\n");

        // close(g_sockfd);
        // int t = connectToServ();
        // if (t != 0)
        //     g_sockfd = t;

        // sleep(1);
        // printf("reconnect succeed.\n");
    }
    //printf("sock: %d\n", g_sockfd);
    printf("rdata: %d\n", n);
}
void sendAllRData(hashtable_t* h)
{
    printf("\n\n\n send full data !!\n\n\n");

    int i;
    void* e;
    for (i = 0; i < h->tablelength; i++) {
        e = h->table[i];
        while (NULL != e) {
            printf("sending full data ....\n");
            sendRDataMsg((RData_MsgContent*)e, g_sockfd);
            //sleep(2);
            e = (void*)*(unsigned long*)(e + h->offset);
        }
    }
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

    int sockfd;
    int isConn = 0;
    while (isConn == 0) {

        sockfd = Socket(AF_INET, SOCK_STREAM, 0);

        //int bufsize = 1024;
        //socklen_t len= sizeof(bufsize);
        //setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,&bufsize,len);

        //printf("%d\n",bufsize );

        //getsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,&bufsize,&len);

        //printf("%d\n",bufsize );

        servaddr = initSockAddr(server_ip, server_port); //move it ???

        signal(SIGPIPE, SIG_IGN); /// move it to the beginning of roamClient() ???

        printf("connecting to server ...\n");
        if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
            printf("connect to server failed. trying to reconnect... \n");
            close(sockfd);
            sleep(waitReconn);
        }
        else{
            isConn = 1;
            printf("connect succeed.\n");
        }
    }

    return sockfd;
}
void* roamClient()
{
     int t = connectToServ();
    // if (t != 0)
         g_sockfd = t;
    // else
    //     return NULL;

    alarmHandler();

    rdtable = hashtable_create(1000, sizeof(RData_MsgContent), 0, 0, rd_free, rd_hash, rd_compare);

    //int len = getjson();
    getFromRabbit(rdtable);

    //call select to check if g_sockfd is readable or writable.

    //jsonStrParse(jsontest, len , rdtable);
    //printf("end!\n");

    //while (1) {
    //sleep(1);
    //sendRDataMsg(g_sockfd);
    // RData_MsgContent* rdata1 = j2s(test_json1);
    // RData_MsgContent* rdata2 = j2s(test_json2);

    // int i;
    // for(i=0;i<sizeof(RData_MsgContent);i++)
    //     printf("%d",*((char *)rdata1+i) );
    // printf("\n");
    // if(!hashtable_search(rdtable,rdata1)){
    //     printf("insert a record!\n");
    //     hashtable_insert(rdtable,rdata1);

    // }
    // printf("hash count: %d\n",hashtable_count(rdtable));

    // if(!hashtable_search(rdtable,rdata2)){
    //     printf("insert a record\n");
    //     hashtable_insert(rdtable,rdata2);

    // }
    // printf("hash count: %d\n",hashtable_count(rdtable));
    // hashtable_trace(rdtable);
    //}
    return NULL;
}





