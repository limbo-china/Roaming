/**************************************************************************/

/* 从配置文件中读取参数配置信息                                           */
/* 配置文件规则：                                                         */
/*              每行仅一个参数值                                          */
/*              行首字符为 # 号的行为注释行                               */
/*              如果一行中不包含 = 号 则默认该行为注释行                  */
/*              参数名字与参数值用 = 号连接                               */
/*              参数名称中不能包含 = 号                                   */
/*               = 号前后不能有空格                                       */
/*              每行最大字符数为 LINE_MAX_LENGTH                          */
/*              参数名最大字符数为 NAME_MAX_LENGTH,超长将丢弃该参数       */
/*              参数值最大字符数为 VALUE_MAX_LENGTH,超长将截断            */
/**************************************************************************/
#include "getconfig.h"

/***************************************************************************/
/* 从配置文件中读取单独一个参数对应的值                                    */
/* 参数：1,配置文件路径; 2,匹配名称; 3,输出存储空间                        */
/* 返回：0,未找到; 1,找到符合名称的值                                      */
/***************************************************************************/
int getConfigValue(const char* conf_path, const char* conf_name,
    char* config_buff)
{
    char config_linebuf[LINE_MAX_LENGTH];
    char line_name[NAME_MAX_LENGTH];
    char* config_sign = "=";
    char* leave_line;
    FILE* f;
    int flag = 0, len;

    f = fopen(conf_path, "r");
    if (f == NULL) {
        printf("OPEN CONFIG FALID:%s\n", conf_path);
        return 0;
    }

    fseek(f, 0, SEEK_SET);
    while (fgets(config_linebuf, LINE_MAX_LENGTH, f) != NULL) {
        if (strlen(config_linebuf) < 4) { //去除空行 "=\r\n"
            continue;
        }

        if (config_linebuf[0] == '#') { //去除注释行 "#"
            continue;
        }

        if (config_linebuf[strlen(config_linebuf) - 1] == 10) {
            config_linebuf[strlen(config_linebuf) - 1] = '\0';
        }

        memset(line_name, 0, sizeof(line_name));
        leave_line = strstr(config_linebuf, config_sign);
        if (leave_line == NULL) { //去除无"="的情况
            continue;
        }

        int leave_num = leave_line - config_linebuf;
        if (leave_num > NAME_MAX_LENGTH) {
            continue;
        }

        strncpy(line_name, config_linebuf, leave_num);
        if (strcmp(line_name, conf_name) == 0) {
            len = strlen(config_linebuf) - leave_num - 1;
            len = len > VALUE_MAX_LENGTH ? VALUE_MAX_LENGTH : len;
            strncpy(config_buff, config_linebuf + (leave_num + 1),
                strlen(config_linebuf) - leave_num - 1);
            *(config_buff + len) = '\0';
            flag = 1;
            break;
        }

        if (fgetc(f) == EOF){
            break;
        }

        fseek(f, -1, SEEK_CUR);
        memset(config_linebuf, 0, sizeof(config_linebuf));
    }

    fclose(f);
    return flag;
}
