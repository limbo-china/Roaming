#ifndef GETRABBIT_H_
#define GETRABBIT_H_

#include "getconfig.h"
#include "client.h"
#include "hashtable.h"
#include "jsontostruct.h"
#include <stdlib.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <assert.h>
#include "utils.h"

void getFromRabbit(hashtable_t *rdtable);  //receive data from rabbit and parse it into hashtable.
int getRabbitCfg(char *_host, int *_port, char *_user, char *_pwd,
	char *_exchgName, char* _key, char *_type);//成功返回1，失败返回0

#endif