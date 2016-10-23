#include "msgstruct.h"

//static u_longlong htonll(u_longlong n); //unsigned long long 网络字节序转换

FRep_MsgContent constructFRepMsg()
{
    FRep_MsgContent frep;

    memset(&frep, 0, sizeof(FRep_MsgContent));

    frep.msg_length = 0x02;
    frep.msg_type = 0x02;

    return frep;
}
// RData_MsgContent constructRDataMsg(u_char _prov, u_short _region, u_longlong _number, u_int _time, u_char _action){
//     RData_MsgContent rdata;

//     memset(&rdata, 0, sizeof(RData_MsgContent));

//     rdata.length = 0x12;
//     rdata.type = 0x03;
//     rdata.roamprovince = _prov;

//     _region = htons(_region);
//     memcpy(rdata.region, &_region, 2);
//     _number = htonll(_number);
//     memcpy(rdata.usernumber, &_number, 8);
//     _time = htonl(_time);
//     memcpy(rdata.time, &_time, 4);

//     rdata.action = _action;

//     return rdata;
// }
FData_FinMsgContent constructFDataFinMsg()
{
    FData_FinMsgContent fdfin;

    memset(&fdfin, 0, sizeof(FData_FinMsgContent));

    fdfin.msg_length = 0x02;
    fdfin.msg_type = 0x04;

    return fdfin;
}
HB_MsgContent constructHBMsg()
{
    HB_MsgContent hb;

    memset(&hb, 0, sizeof(HB_MsgContent));

    hb.msg_length = 0x02;
    hb.msg_type = 0x05;
    return hb;
}
