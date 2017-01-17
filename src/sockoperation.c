#include "sockoperation.h"

// int Socket(int family, int type, int protocol){
//     int n;

//     if ((n = socket(family, type, protocol)) < 0){
//         err_sys("socket error");
//     }

//     return n;
// }
// void Bind(int _sock, const struct sockaddr *sa, socklen_t salen){
//     if (bind(_sock, sa, salen) < 0){
//         err_sys("bind error");
//     }
// }
// int Accept(int _sock, struct sockaddr *sa, socklen_t *salenptr){
//     int n;

//     if ((n = accept(_sock, sa, salenptr)) < 0){
//         err_sys("accept error");
//     }

//     return n;
// }
// void Connect(int _sock, const struct sockaddr *sa, socklen_t salen){
//     if (connect(_sock, sa, salen) < 0){
//         err_sys("connect error");
//     }
// }
// void Listen(int _sock, int backlog){
//     char *ptr;

//     if ((ptr = getenv("LISTENQ")) != NULL){
//         backlog = atoi(ptr);
//     }

//     if (listen(_sock, backlog) < 0){
//         err_sys("listen error");
//     }
// }
// ssize_t sendn(int _sock, const void* vptr, size_t n)
// {
//     size_t nleft;
//     ssize_t nwritten;
//     const char *ptr;

//     ptr = vptr;
//     nleft = n;
//     while (nleft > 0){
//         if ((nwritten = send(_sock, ptr, nleft, 0)) <= 0){
//             if ((nwritten < 0) && (errno == EINTR)){
//                 nwritten = 0;
//             } else {
//                 return -1;
//             }
//         }

//         nleft -= nwritten;
//         ptr += nwritten;
//     }

//     return n;
// }
int getSrvCfg(Connection_Info *_conn){
    if(_conn->type == SMSCONN){
        if (!getConfigValue(CONFIG_PATH, "sms_server_ip", _conn->server_ip)){
            log_error(_conn->log, "Get sms server ip failed.\n");
            return 0;
        }

        char _p[10];
        if (!getConfigValue(CONFIG_PATH, "sms_server_port", _p)){
            log_error(_conn->log, "Get sms server port failed.\n");
            return 0;
        }

        _conn->server_port = atoi(_p);
        return 1;
    }
    else if(_conn->type == MMSCONN){
        if (!getConfigValue(CONFIG_PATH, "mms_server_ip", _conn->server_ip)){
            log_error(_conn->log, "Get mms server ip failed.\n");
            return 0;
        }

        char _p[10];
        if (!getConfigValue(CONFIG_PATH, "mms_server_port", _p)){
            log_error(_conn->log, "Get mms server port failed.\n");
            return 0;
        }

        _conn->server_port = atoi(_p);
        return 1;
    }
    else{
        log_error(_conn->log, "Unknown connection type.\n");
        return 0;
    }
}
struct sockaddr_in initSockAddr(char *_ip, int _port){
    struct sockaddr_in sockAddr;

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = (_ip == NULL) ? htonl(INADDR_ANY) : inet_addr(_ip);
    sockAddr.sin_port = htons(_port);
    return sockAddr;
}
