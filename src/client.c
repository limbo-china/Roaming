#include "client.h"
#include "jsontostruct.h"
#include "datahash.h"


const int tr2KeepAlive = 3;
const int waitReconn = 5;
int g_sockfd = 0;

const char* test_json1 = "{\"RoamProvince\": \"31\",\"Region\": \"21512\",\"HomeCode\":\"0353\",\"UserNumber\": \"8613899050320\",\"Time\": \"132817183113\",\"Action\": \"1\"}";
const char* test_json2 = "{\"RoamProvince\": \"32\",\"Region\": \"41112\",\"HomeCode\":\"0353\",\"UserNumber\": \"8613899050320\",\"Time\": \"13281283113\",\"Action\": \"1\"}";
const char* test_json3 = "{\"Result\":[{\"RoamProvince\":\"34\",\"Region\":\"370000\",\"HomeCode\":\"0353\",\"UserNumber\":\"8613835330813\",\"Time\":\"1477016938\",\"Action\":\"0\"},{\"RoamProvince\":\"37\",\"Region\":\"370000\",\"HomeCode\":\"0353\",\"UserNumber\":\"8613835330812\",\"Time\":\"1477016938\",\"Action\":\"1\"}]}";
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

    //sendHBMsg(g_sockfd); //发送心跳包进行检测

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
        if (t != 0)
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
void sendRDataMsg(RData_MsgContent* rdata, int _sock)
{
    int n;
    int i;
    for(i=0;i<sizeof(RData_MsgContent);i++)
        printf("%d",*((char*)rdata+i));
    printf("\n");
    // if ((n = send(_sock, rdata, sizeof(RData_MsgContent), 0)) < 0) {
    //     printf("connection broken!waitting for reconnecting ...\n");
    //     sleep(waitReconn); // wait for reconnecting
    //     //
    // }
    // printf("rdata: %d\n", n);
}
void sendAllRData(hashtable_t *h){
    int i;
    void *e;
    for (i = 0; i < h->tablelength; i++)
    {
        e = h->table[i];
        while (NULL != e){
            sendRDataMsg((RData_MsgContent *)e, g_sockfd);
            e = (void *) *(unsigned long *) (e + h->offset);
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

    hashtable_t *rdtable = hashtable_create(20,sizeof(RData_MsgContent),0,0, rd_free, rd_hash, rd_compare);

    jsonStrParse(test_json3,rdtable);

    sendAllRData(rdtable);
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





