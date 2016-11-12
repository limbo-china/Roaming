#ifndef CLIENT_H_
#define CLIENT_H_

#include "sockoperation.h"
#include "msgstruct.h"
#include "hashtable.h"
#include "datahash.h"

#include "rabbit_test.h"
#include "getrabbit.h"

extern int g_sockfd;
extern pthread_mutex_t send_mutex;
void* requestDetect();
void* roamClient();
void alarmHandler();
void sendHBMsg(int _sock);
void sendFRepMsg(int _sock);
void sendRDataMsg(RData_MsgContent* rdata,int _sock);
void sendFDataFinMsg(int _sock);
void sendFullRData(hashtable_t *rdtable, u_char prov);
int connectToServ();  // connect to the server and return the socket descriptor
#endif