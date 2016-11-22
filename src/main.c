#include "client.h"

#define LOG_PATH "./log"

log_t * g_log;

int main()
{
    pthread_t queueThread;
    pthread_t clientThread;
    //pthread_t dumpThread;
    int ret = 0;

    g_log = log_create(LOG_PATH, "LOG", Log_debug);

    ret = pthread_create(&clientThread, NULL, roamClient, NULL);
    if (ret) {
        log_error(g_log,"Client pthread_create error:(return code %d)!", ret);
        return 0;
    }

    //sleep(1);

    ret = pthread_create(&queueThread, NULL, queueFromRabbit, NULL);
    if (ret) {
        log_error(g_log,"Request pthread_create error:(return code %d)!", ret);
        return 0;
    }

    // ret = pthread_create(&dumpThread, NULL, hashTableDump , NULL);
    // if(ret){
    //     printf("dump pthread_create error:(return code %d)!", ret);
    //     return 0;
    // }

    pthread_join(clientThread, NULL);
    pthread_join(queueThread, NULL);
    //pthread_join(dumpThread,NULL);
    return 0;
}