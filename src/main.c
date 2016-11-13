#include "client.h"

int main()
{
    pthread_t reqThread;
    pthread_t clientThread;
    pthread_t dumpThread;
    int ret = 0;

    ret = pthread_create(&clientThread, NULL, roamClient, NULL);
    if (ret) {
        printf("client pthread_create error:(return code %d)!", ret);
        return 0;
    }

    sleep(1);

    ret = pthread_create(&reqThread, NULL, requestDetect, NULL);
    if (ret) {
        printf("request pthread_create error:(return code %d)!", ret);
        return 0;
    }

    ret = pthread_create(&dumpThread, NULL, hashTableDump , NULL);
    if(ret){
        printf("dump pthread_create error:(return code %d)!", ret);
        return 0;
    }

    pthread_join(clientThread, NULL);
    pthread_join(reqThread, NULL);
    pthread_join(dumpThread,NULL);
    return 0;
}