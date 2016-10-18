#ifndef ERROR_H_
#define ERROR_H_

#define MAXLINE 100

#include    <stdarg.h>
#include    <syslog.h>
#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <errno.h>

void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_dump(const char *fmt, ...);
void err_msg(const char *fmt, ...);
void err_quit(const char *fmt, ...);

#endif
