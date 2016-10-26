#ifndef _GETCONFIG__H_
#define _GETCONFIG__H_

#include <stdio.h>
#include <string.h>

#define LINE_MAX_LENGTH 256 //每行最大字符数
#define NAME_MAX_LENGTH 50 //每个参数名最大字符数
#define VALUE_MAX_LENGTH 50 //每个参数值最大字符数

/***************************************************************************/
/* 从配置文件中读取单独一个参数对应的值                                    */
/* 参数：1,配置文件路径; 2,匹配名称; 3,输出存储空间                        */
/* 返回：0,未找到; 1,找到符合名称的值                                      */
/***************************************************************************/
int getConfigValue(const char *conf_path, const char *conf_name, char *config_buff);

#define CONFIG_PATH "./conf/roam.conf"

#endif
