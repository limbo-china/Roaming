#ifndef SOCKOPERATION_H_
#define SOCKOPERATION_H_

#include "sockbase.h"
#include "getconfig.h"
#include "error.h"

//
//-------socket 包裹函数----------//
int				   Socket(int family, int type, int protocol);
void			   Bind(int _sock, const struct sockaddr *sa, socklen_t salen);
int				   Accept(int _sock, struct sockaddr *sa, socklen_t *salenptr);
void			   Connect(int _sock, const struct sockaddr *sa, socklen_t salen);
void			   Listen(int _sock, int backlog);
ssize_t			   sendn(int fd, const void *vptr, size_t n); //发送n个字节大小的数据,成功返回发送的字节数,失败返回-1
//-----------------------------//

int				   getSrvCfg(char *_ip, int *_port); //获取服务器ip和端口,成功返回1，失败返回0
struct sockaddr_in initSockAddr(char *_ip, int _port); //返回一个指定ip和端口的sockaddr

#endif
