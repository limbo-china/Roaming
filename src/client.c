#include "client.h"

#define DUMP_FILE_PATH "hashtable.dmp"

const double tr2KeepAlive = 60.0;
const double sendLogTimeval = 30.0;
const int waitReconn = 5;
const int waitRequest = 2;
const int dumpWriteTimeval = 1800;

int g_sockfd = 0;
//pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
hashtable_t* rdtable = NULL;
rfifo_t *rdqueue = NULL;

time_t lastlogtime = 0 , lastsendtime =0;
int leavenum =0 ,enternum =0;
int isConn = 0;

//int datanum = 0;

void requestDetect()
{
    int maxfdp1;
    fd_set rset; // read set and write set
    ssize_t n;

    FD_ZERO(&rset);

    struct timeval timeout;  

    if(g_sockfd != 0){

        FD_SET(g_sockfd, &rset);
        maxfdp1 = g_sockfd + 1;
        timeout.tv_sec = waitRequest;
        timeout.tv_usec = 0;

        log_info(g_log,"Waitting for requst from server...\n");
        if(select(maxfdp1, &rset, NULL, NULL, &timeout) == 0){
            log_info(g_log,"Waitting timeout.\n");
        }

        if (FD_ISSET(g_sockfd, &rset)) {
            

            FReq_MsgContent* freqmsg = (FReq_MsgContent*)malloc(sizeof(FReq_MsgContent));
            if ((n = recv(g_sockfd, freqmsg, sizeof(FReq_MsgContent), 0)) == 3) {
                if (freqmsg->msg_length == 3 && freqmsg->msg_type == 1) {
                    log_info(g_log,"Received REQUEST MESSAGE.\n");

                    if(sendFRepMsg(g_sockfd) == 0)
                        return ;
                    if(sendFullRData(rdtable, freqmsg->dest_prov) == 0)
                        return ;
                    if(sendFDataFinMsg(g_sockfd) == 0)
                        return ;
                }
            }
            free(freqmsg);
        }
    }
}
// void alarmHandler()
// {
//     if (g_sockfd == 0) {
//         signal(SIGALRM, alarmHandler);
//         alarm(tr2KeepAlive);
//         return;
//     }

//     sendHBMsg(g_sockfd); //发送心跳包进行检测

//     signal(SIGALRM, alarmHandler); //重新定时
//     alarm(tr2KeepAlive);
// }
int sendHBMsg(int _sock)
{
    HB_MsgContent hbmsg = constructHBMsg();

    int n;
    if ((n = send(_sock, &hbmsg, sizeof(hbmsg), 0)) < 0) {
        //
        /////reconnect
        log_info(g_log,"Connection broken! Waitting for reconnecting ...\n");
        log_info(g_log,"Reconnecting to server ...\n");
        close(g_sockfd);
        isConn = 0;
        connectToServ();

        return 0;
    }
    log_info(g_log,"Send HB MESSAGE successfully.\n");
    return 1;
}
int sendFRepMsg(int _sock)
{
    FRep_MsgContent frepmsg = constructFRepMsg();

    int n;
    if ((n = send(_sock, &frepmsg, sizeof(frepmsg), 0)) < 0) {
        log_info(g_log,"Connection broken! Waitting for reconnecting ...\n");
        log_info(g_log,"Reconnecting to server ...\n");
        close(g_sockfd);
        isConn = 0;
        connectToServ();

        return 0;
    }
    log_info(g_log,"Send REPLY MESSAGE successfully.\n");
    return 1;
}
int sendFDataFinMsg(int _sock)
{
    FData_FinMsgContent fdatafinmsg = constructFDataFinMsg();

    int n;
    if ((n = send(_sock, &fdatafinmsg, sizeof(fdatafinmsg), 0)) < 0) {
        log_info(g_log,"Connection broken! Waitting for reconnecting ...\n");
        log_info(g_log,"Reconnecting to server ...\n");
        close(g_sockfd);
        isConn = 0;
        connectToServ();

        return 0;
    }
    log_info(g_log,"Send FIN MESSAGE successfully.\n");
    return 1;
}
int sendRDataMsg(RData_MsgContent* rdata, int _sock)
{
    int n;
    // datanum++;
    // int i;
    // printf("%d: ", datanum);
    // printf("%u", rdata->length);
    // printf("%u", rdata->type);
    // printf("%u", rdata->roamprovince);
    // printf("%u",  *((u_char *)&rdata->region));
    // printf("%u",  *((u_char *)&rdata->region+1));
    // for (i = 0; i < 12; i++)
    //     printf("%c", *((u_char*)rdata->usernumber + i));
    // printf("%u", ntohl(rdata->time));
    // printf("%d", rdata->action);
    // printf("\n");

    char _datamsg[RDATAMSG_LENGTH + 1];
    _datamsg[0] = rdata->length;
    _datamsg[1] = rdata->type;
    _datamsg[2] = rdata->roamprovince;
    strncpy(_datamsg + 3, (char*)&rdata->region, 2);
    strncpy(_datamsg + 5, rdata->usernumber, 12);
    strncpy(_datamsg + 17, (char*)&rdata->time, 4);
    _datamsg[21] = rdata->action;

    if ((n = send(_sock, _datamsg, RDATAMSG_LENGTH, 0)) < 0) {
        log_info(g_log,"Connection broken! Waitting for reconnecting ...\n");

        log_info(g_log,"Reconnecting to server ...\n");

        close(g_sockfd);
        isConn = 0;
        connectToServ();

        return 0;
    }
    return 1;
}
int sendFullRData(hashtable_t* h, u_char prov)
{
    log_info(g_log,"Start to send FULL DATA.\n");

    int i,sentNum=0;
    void* e;
    for (i = 0; i < h->tablelength; i++) {
        e = h->table[i];
        while (NULL != e) {
            
            RData_MsgContent* rdptr = (RData_MsgContent*)e;
            if(prov == 0 || prov == rdptr->roamprovince){
                if(sendRDataMsg(rdptr, g_sockfd) == 0)
                    return 0;
                sentNum = sentNum +1;
            }
            e = (void*)*(unsigned long*)(e + h->offset);
        }
    }
    log_info(g_log,"Sending FULL DATA end. %d msg sent.\n", sentNum);
    return 1;
}
void connectToServ()
{

    char server_ip[20] = { 0 };
    int server_port = 0;
    struct sockaddr_in servaddr;

    if (!getSrvCfg(server_ip, &server_port)) {
        log_error(g_log,"Get server configuration failed.\n");
        return ;
    }

    int sockfd;
    isConn = 0;
    while (isConn == 0) {

        sockfd = Socket(AF_INET, SOCK_STREAM, 0);

        servaddr = initSockAddr(server_ip, server_port); //move it ???

        signal(SIGPIPE, SIG_IGN); /// move it to the beginning of roamClient() ???

        log_info(g_log, "Connecting to server %s:%d ...\n", server_ip, server_port);
        if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
            log_info(g_log, "Connect to server %s:%d failed. Trying to reconnect... \n", server_ip, server_port);
            close(sockfd);
            sleep(waitReconn);
        }
        else{
            log_info(g_log, "Connect to server %s:%d successfully.\n", server_ip, server_port);

            isConn = 1;
            g_sockfd = sockfd;
            requestDetect(); //wait for request
        }
    }
}
void processRData(RData_MsgContent* rdata){

    if (*rdata->usernumber != 0) { // data without a usernumber will not be considered.

                // if(hashtable_search(rdtable,rdata) == NULL){
                //     printf("insert a record!\n");
                //     hashtable_insert(rdtable, rdata);
                //     sendRDataMsg(rdata, g_sockfd);
                // }
                // else{
                //     if(rdata->action == 0){
                //         hashtable_remove(rdtable,rdata);         
                //     }
                //     else{
                //         ////
                //         //...
                //         hashtable_remove(rdtable,rdata);  
                //         hashtable_insert(rdtable,rdata);
                //     }
                //     sendRDataMsg(rdata, g_sockfd);
                // }
                RData_MsgContent* t = (RData_MsgContent*)malloc(sizeof(RData_MsgContent));
                memcpy(t, rdata, sizeof(RData_MsgContent));

                if(rdata->action == 0){
                    if(hashtable_search(rdtable,rdata) != NULL ){
                        //printf("remove from table when leave.\n");
                        hashtable_remove(rdtable,rdata);
                    }
                    //printf("send leave msg.\n");
                    free(rdata);
                    //pthread_mutex_lock(&queue_mutex);
                    if(isConn)
                        rfifo_put(rdqueue,t);
                    //pthread_mutex_unlock(&queue_mutex);
                }
                else{
                    RData_MsgContent* rdptr = (RData_MsgContent*)hashtable_search(rdtable,rdata);
                    if(rdptr == NULL){
                        //printf("insert into table when enter.\n");
                        hashtable_insert(rdtable,rdata);
                    }
                    else{
                        //printf("remove origin and send leave msg when enter.\n");
                        rdptr->action = 0;
                        if(isConn){
                            memcpy(t, rdptr, sizeof(RData_MsgContent));
                            rfifo_put(rdqueue,t);
                        }
                        hashtable_remove(rdtable,rdata);
                        //printf("insert into table when enter.\n");
                        hashtable_insert(rdtable,rdata);
                    }
                    //printf("send enter msg.\n");
                    if(isConn)
                        rfifo_put(rdqueue,t);
                }
    } 
    //printf("queue count: %d\n", rfifo_count(rdqueue));
}
void* roamClient()
{   

    connectToServ();

    //alarmHandler();

    lastlogtime = time(NULL);
    lastsendtime = time(NULL);

    for(;;){
        
        if (isConn == 1){
            RData_MsgContent* t = (RData_MsgContent*)rfifo_get(rdqueue);

            if(t == NULL){
            //printf("get queue error.\n"); 
                if(difftime(time(NULL),lastsendtime) >= tr2KeepAlive){ //send hb msg
                    sendHBMsg(g_sockfd);
                    lastsendtime = time(NULL);
                }
                usleep(1000000);
            }            
            else{

                if(difftime(time(NULL),lastlogtime) >= sendLogTimeval){
                    log_info(g_log,"Send %d msg totally in %d secs.[LEAVE: %d, ENTER: %d][TCount: %d, QCount: %d]\n", 
                        leavenum+enternum, (int)(difftime(time(NULL),lastlogtime)),leavenum,enternum,
                        hashtable_count(rdtable), rfifo_count(rdqueue));
                    lastlogtime = time(NULL);
                    leavenum = 0;
                    enternum = 0;
                }

                if(t->action == 0)
                    leavenum++;
                else if(t->action == 1)
                    enternum++;

                sendRDataMsg(t, g_sockfd); 

                lastsendtime = time(NULL);

                //free(t);             

            }
            //printf("queue count: %d.\n",rfifo_count(rdqueue));
            //sleep(1);
        }
        else{
            usleep(2000000);
        }

        
    }

    return NULL;
}
void* queueFromRabbit(){

    //int len = getjson();
    for(; ;){
        
        getFromRabbit();
        //jsonStrParse(jsontest, len , rdqueue);
        sleep(5);
    }
    return NULL;

}

void *hashTableDump(){
    FILE* f;
    f = fopen(DUMP_FILE_PATH, "w+");

    if (f == NULL) {
        log_info(g_log, "Open DUMP failed:%s\n", DUMP_FILE_PATH);
        return 0;
    }

    for(; ;){
        sleep(dumpWriteTimeval);
        fseek(f, 0, SEEK_SET);
        if(rdtable != NULL)
            dumpWriteUpdate(f);
    }
}

void dumpWriteUpdate(FILE *f){
    log_info(g_log, "Updating the dumpfile.\n");

    int i, writeNum =0;
    void* e;
    for (i = 0; i < rdtable->tablelength; i++) {
        e = rdtable->table[i];
        while (NULL != e) {
            RData_MsgContent* rdptr = (RData_MsgContent*)e;
            fwrite(rdptr, sizeof(RData_MsgContent), 1, f);
            writeNum = writeNum +1;
            //fputc('\n',f);
            e = (void*)*(unsigned long*)(e + rdtable->offset);
        }
    }
    fflush(f);
    log_info(g_log, "Dumpfile updated. %d records written.[TCount: %d, QCount: %d]\n", writeNum,hashtable_count(rdtable), rfifo_count(rdqueue));
}

void dumpFileRead(){
    FILE* f;
    f = fopen(DUMP_FILE_PATH, "r");
    int readNum =0;

    if (f == NULL) {
        log_info(g_log, "Open DUMP failed:%s\n", DUMP_FILE_PATH);
        return ;
    }

    

    log_info(g_log, "Reading the dumpfile.\n");

    while(1){

        RData_MsgContent* t = (RData_MsgContent*)malloc(sizeof(RData_MsgContent));

        if(fread(t, sizeof(RData_MsgContent), 1, f) != 1)
            break;

        readNum = readNum +1;
        processRData(t);
    }

    log_info(g_log, "Read successfully. %d records got.\n",readNum);


}





