#ifndef CLIENT_H_
#define CLIENT_H_

#include "sockoperation.h"
#include "msgstruct.h"
#include "hashtable.h"

extern int g_sockfd;

void* heartBeatDetect();
void* roamClient();
void alarmHandler();
void sendHBMsg(int _sock);
void sendFRepMsg(int _sock);
void sendRDataMsg(RData_MsgContent* rdata,int _sock);
void sendAllRData(hashtable_t *rdtable);
int connectToServ();  // connect to the server and return the socket descriptor
#endif