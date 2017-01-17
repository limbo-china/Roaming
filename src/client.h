#ifndef CLIENT_H_
#define CLIENT_H_

#include "msgstruct.h"
#include "hashtable.h"
#include "datahash.h"
#include "rfifo.h"

#include "rabbit_test.h"
#include "getrabbit.h"
#include "log.h"

enum {SMSCONN=1,MMSCONN=2};

typedef struct Connection{
	int type;
  	int socket;
  	log_t * log;
  	rfifo_t *rdqueue;
  	time_t lastlogtime; 
  	time_t lastsendtime;
	int leavenum;
	int enternum;
	int isConn;
	char server_ip[20];
	int server_port;
} Connection_Info;

#include "sockoperation.h"

//extern int g_sockfd;
//extern rfifo_t *rdqueue;
//extern pthread_mutex_t send_mutex;
extern hashtable_t* rdtable;
extern Connection_Info *sms_conn;
extern Connection_Info *mms_conn;
//extern log_t * g_log;

void requestDetect(Connection_Info *_conn);
void* roamClient(void *_conn_t);
void* queueFromRabbit();
void* hashTableDump();
//void alarmHandler();
void processRData(RData_MsgContent* rdata);
int sendHBMsg(Connection_Info *_conn);  //1 success , 0 fail.
int sendFRepMsg(Connection_Info *_conn);
int sendRDataMsg(RData_MsgContent* rdata,Connection_Info *_conn);
int sendFDataFinMsg(Connection_Info *_conn);
int sendFullRData(hashtable_t *rdtable, u_char prov, Connection_Info *_conn);
void connectToServ(Connection_Info *_conn);  // connect to the server 
void dumpWriteUpdate(FILE *f); //update the hashtable dumpfile.
void dumpFileRead(); // read dumpfile and restore the hashtable.


#endif