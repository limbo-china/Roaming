#include "client.h"

#define LOG_PATH "./log"

log_t * g_log;

int main()
{
    pthread_t queueThread;
    pthread_t clientThread;
    pthread_t dumpThread;
    int ret = 0;


    //日志拓展到2个，一个为SMS，一个为MMS。
    g_log = log_create(LOG_PATH, "LOG", Log_debug);
    rdtable = hashtable_create(1000, sizeof(RData_MsgContent), 0, 1, rd_free, rd_hash, rd_compare);
    
    //需要2个队列。
    rdqueue = rfifo_create(30000,NULL);

    dumpFileRead();


    //这里需要通过传参数，创建两个不同的连接发送线程。每个线程分别处理一个队列。
    ret = pthread_create(&clientThread, NULL, roamClient, NULL);
    if (ret) {
        log_error(g_log,"Client pthread_create error:(return code %d)!", ret);
        return 0;
    }

    //sleep(1);

    //一个处理队列的线程就够,但是此线程需要将数据放入2个队列里。
    ret = pthread_create(&queueThread, NULL, queueFromRabbit, NULL);
    if (ret) {
        log_error(g_log,"Queue pthread_create error:(return code %d)!", ret);
        return 0;
    }

    ret = pthread_create(&dumpThread, NULL, hashTableDump , NULL);
    if(ret){
        log_error(g_log,"Dump pthread_create error:(return code %d)!", ret);
        return 0;
    }

    pthread_join(clientThread, NULL);
    pthread_join(queueThread, NULL);
    pthread_join(dumpThread,NULL);
    return 0;
}