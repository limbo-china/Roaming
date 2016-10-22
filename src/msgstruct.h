#ifndef MSGSTRUCT_H_
#define MSGSTRUCT_H_

#include <string.h>
#include "sockbase.h"

typedef unsigned char      u_char;
typedef unsigned short     u_short;
typedef unsigned int       u_int;
typedef unsigned long long u_longlong;

#define RDATAMSG_LENGTH 29

typedef struct F_REQ_MSG_CONTENT {
    u_char msg_length;
    u_char msg_type;
    u_char dest_prov;
}FReq_MsgContent;

typedef struct F_REP_MEG_CONTENT {
    u_char msg_length;
    u_char msg_type;
}FRep_MsgContent;

typedef struct R_DATA_MEG_CONTENT_T {
    char roamprovince[2];
    char region[6];
    char usernumber[13];
    u_int time;
    u_char action;
}RData_MsgContent_T;

typedef struct R_DATA_MEG_CONTENT {
    u_char length;
    u_char type;
    u_char roamprovince;
    u_short region;
    char usernumber[13];
    u_int time;
    u_char action;

    void*  next;
}RData_MsgContent;

typedef struct F_DATA_FIN_MEG_CONTENT {
    u_char msg_length;
    u_char msg_type;
}FData_FinMsgContent;

typedef struct HB_MSG_CONTENT {
    u_char msg_length;
    u_char msg_type;
}HB_MsgContent;

FReq_MsgContent     constructFReqMsg(u_char _prov);
FRep_MsgContent     constructFRepMsg();
FData_FinMsgContent constructFDataFinMsg();
HB_MsgContent       constructHBMsg();

#endif
