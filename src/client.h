#ifndef CLIENT_H_
#define CLIENT_H_

#include "sockoperation.h"
#include "msgstruct.h"
#include "hashtable.h"
#include "datahash.h"
#include "rfifo.h"

#include "rabbit_test.h"
#include "getrabbit.h"
#include "log.h"

extern int g_sockfd;
extern rfifo_t *rdqueue;
extern pthread_mutex_t send_mutex;
extern log_t * g_log;
void requestDetect();
void* roamClient();
void* queueFromRabbit();
void* hashTableDump();
void alarmHandler();
void processRData(RData_MsgContent* rdata);
void sendHBMsg(int _sock);
void sendFRepMsg(int _sock);
void sendRDataMsg(RData_MsgContent* rdata,int _sock);
void sendFDataFinMsg(int _sock);
void sendFullRData(hashtable_t *rdtable, u_char prov);
void connectToServ();  // connect to the server 
void dumpWriteUpdate(FILE *f); //update the hashtable dumpfile.

#endif