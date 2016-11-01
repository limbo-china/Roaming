#include "client.h"

int main()
{
    pthread_t reqThread;
    pthread_t clientThread;
    int ret = 0;

    ret = pthread_create(&clientThread, NULL, roamClient, NULL);
    if (ret) {
        printf("client pthread_create error:(return code %d)!", ret);
        return 0;
    }

    sleep(3);

    ret = pthread_create(&reqThread, NULL, requestDetect, NULL);
    if (ret) {
        printf("request pthread_create error:(return code %d)!", ret);
        return 0;
    }

    pthread_join(clientThread, NULL);

    pthread_join(reqThread, NULL);
    return 0;
}