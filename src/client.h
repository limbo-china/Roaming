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
extern hashtable_t* rdtable;
extern log_t * g_log;

void requestDetect();
void* roamClient();
void* queueFromRabbit();
void* hashTableDump();
//void alarmHandler();
void processRData(RData_MsgContent* rdata);
int sendHBMsg(int _sock);  //1 success , 0 fail.
int sendFRepMsg(int _sock);
int sendRDataMsg(RData_MsgContent* rdata,int _sock);
int sendFDataFinMsg(int _sock);
int sendFullRData(hashtable_t *rdtable, u_char prov);
void connectToServ();  // connect to the server 
void dumpWriteUpdate(FILE *f); //update the hashtable dumpfile.
void dumpFileRead(); // read dumpfile and restore the hashtable.

#endif