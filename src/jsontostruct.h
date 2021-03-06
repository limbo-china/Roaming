#ifndef JSONTOSTRUCT_H_
#define JSONTOSTRUCT_H_

#include "msgstruct.h"
#include "s2j.h"
#include "hashtable.h"
#include "client.h"
#include <stdint.h>
#include <stdio.h>

RData_MsgContent* j2s(const char* _json); // get json and transformat it to struct.
void jsonStrParse(const char* jsonstr, int len); //transformat json and put into queue
u_char getProv(u_char _prov);

#endif