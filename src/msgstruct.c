#include "msgstruct.h"

static u_longlong htonll(u_longlong n); //unsigned long long 网络字节序转换

FReq_MsgContent constructFReqMsg(u_char _prov){
    FReq_MsgContent freq;

    memset(&freq, 0, sizeof(FReq_MsgContent));

    freq.msg_length = 0x03;
    freq.msg_type  = 0x01;
    freq.dest_prov = _prov;

    return freq;
}
FRep_MsgContent constructFRepMsg(){
    FRep_MsgContent frep;

    memset(&frep, 0, sizeof(FRep_MsgContent));

    frep.msg_length = 0x02;
    frep.msg_type = 0x02;

    return frep;
}
RData_MsgContent constructRDataMsg(u_char _prov, u_short _city, u_longlong _number, u_int _time, u_char _optype){
    RData_MsgContent rdata;

    memset(&rdata, 0, sizeof(RData_MsgContent));

    rdata.msg_length = 0x12;
    rdata.msg_type = 0x03;
    rdata.province = _prov;

    _city = htons(_city);
    memcpy(rdata.city, &_city, 2);
    _number = htonll(_number);
    memcpy(rdata.number, &_number, 8);
    _time = htonl(_time);
    memcpy(rdata.time, &_time, 4);

    rdata.oper_type = _optype;

    return rdata;
}
FData_FinMsgContent constructFDataFinMsg(){
    FData_FinMsgContent fdfin;

    memset(&fdfin, 0, sizeof(FData_FinMsgContent));

    fdfin.msg_length = 0x02;
    fdfin.msg_type = 0x04;

    return fdfin;
}
HB_MsgContent constructHBMsg(){
    HB_MsgContent hb;

    memset(&hb, 0, sizeof(HB_MsgContent));

    hb.msg_length = 0x02;
    hb.msg_type = 0x05;
    return hb;
}
static u_longlong htonll(u_longlong n){
    u_longlong temp_low, temp_high;

    temp_low  = htonl((unsigned long)n);
    temp_high = htonl((unsigned long)(n >> 32));

    n  &= 0;
    n  |= temp_low;
    n <<= 32;
    n  |= temp_high;
    return n;
}
