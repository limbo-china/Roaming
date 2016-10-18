#ifndef MSGSTRUCT_H_
#define MSGSTRUCT_H_

#include <string.h>
#include "sockbase.h"

typedef unsigned char	   u_char;
typedef unsigned short	   u_short;
typedef unsigned int	   u_int;
typedef unsigned long long u_longlong;

typedef struct F_REQ_MSG_CONTENT {
    u_char msg_length;
    u_char msg_type;
    u_char dest_prov;
}FReq_MsgContent;

typedef struct F_REP_MEG_CONTENT {
    u_char msg_length;
    u_char msg_type;
}FRep_MsgContent;

typedef struct R_DATA_MEG_CONTENT {
    u_char msg_length;
    u_char msg_type;
    u_char province;
    u_char city[2];
    u_char number[8];
    u_char time[4];
    u_char oper_type;
}RData_MsgContent;

typedef struct F_DATA_FIN_MEG_CONTENT {
    u_char msg_length;
    u_char msg_type;
}FData_FinMsgContent;

typedef struct HB_MSG_CONTENT {
    u_char msg_length;
    u_char msg_type;
}HB_MsgContent;

FReq_MsgContent		constructFReqMsg(u_char _prov);
FRep_MsgContent		constructFRepMsg();
RData_MsgContent	constructRDataMsg(u_char _prov, u_short _city, u_longlong _number, u_int _time, u_char _optype);
FData_FinMsgContent constructFDataFinMsg();
HB_MsgContent		constructHBMsg();

#endif
