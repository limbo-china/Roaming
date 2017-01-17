#include "client.h"

#define LOG_PATH "./log"

int main()
{
    pthread_t queueThread;
    pthread_t sms_clientThread;
    pthread_t mms_clientThread;
    pthread_t dumpThread;
    int ret = 0;

    //初始化2个连接
    sms_conn = (Connection_Info *)malloc(sizeof(Connection_Info));
    mms_conn = (Connection_Info *)malloc(sizeof(Connection_Info));
    memset(sms_conn, 0, sizeof(*sms_conn));
    memset(mms_conn, 0, sizeof(*mms_conn));
    sms_conn->type = SMSCONN;
    mms_conn->type = MMSCONN;

    //日志拓展到2个，一个为SMS，一个为MMS。
    sms_conn->log = log_create(LOG_PATH, "SMS_LOG", Log_debug);
    mms_conn->log = log_create(LOG_PATH, "MMS_LOG", Log_debug);

    //hash表仍为一个
    rdtable = hashtable_create(65535*1000, sizeof(RData_MsgContent), 0, 1, rd_free, rd_hash, rd_compare);
    
    //2个队列。
    sms_conn->rdqueue = rfifo_create(30000,NULL);
    mms_conn->rdqueue = rfifo_create(30000,NULL);

    dumpFileRead();


    //通过传参数，创建两个不同的连接发送线程。每个线程分别处理一个队列。
    ret = pthread_create(&sms_clientThread, NULL, roamClient, &sms_conn);
    if (ret) {
        log_error(sms_conn->log,"SMS Client pthread_create error:(return code %d)!", ret);
        return 0;
    }
    ret = pthread_create(&mms_clientThread, NULL, roamClient, &mms_conn);
    if (ret) {
        log_error(mms_conn->log,"MMS Client pthread_create error:(return code %d)!", ret);
        return 0;
    }

    //sleep(1);

    //一个处理队列的线程就够,但是此线程需要将数据放入2个队列里。
    ret = pthread_create(&queueThread, NULL, queueFromRabbit, NULL);
    if (ret) {
        log_error(sms_conn->log,"Queue pthread_create error:(return code %d)!", ret);
        log_error(mms_conn->log,"Queue pthread_create error:(return code %d)!", ret);
        return 0;
    }

    ret = pthread_create(&dumpThread, NULL, hashTableDump , NULL);
    if(ret){
        log_error(sms_conn->log,"Dump pthread_create error:(return code %d)!", ret);
        log_error(mms_conn->log,"Dump pthread_create error:(return code %d)!", ret);
        return 0;
    }

    pthread_join(sms_clientThread, NULL);
    pthread_join(mms_clientThread,NULL);  
    pthread_join(queueThread, NULL);
    pthread_join(dumpThread,NULL);
    return 0;
}