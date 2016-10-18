#ifndef CLIENT_H_
#define CLIENT_H_

#include "sockoperation.h"
#include "msgstruct.h"

extern int g_sockfd;

void *heartBeatDetect();
void *roamClient();
void  alarmHandler();
void  sendHBMsg(int _sock);
void  sendFRepMsg(int _sock);

#endif
