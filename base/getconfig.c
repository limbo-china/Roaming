/**************************************************************************/

/* �������ļ��ж�ȡ����������Ϣ                                           */
/* �����ļ�����                                                         */
/*              ÿ�н�һ������ֵ                                          */
/*              �����ַ�Ϊ # �ŵ���Ϊע����                               */
/*              ���һ���в����� = �� ��Ĭ�ϸ���Ϊע����                  */
/*              �������������ֵ�� = ������                               */
/*              ���������в��ܰ��� = ��                                   */
/*               = ��ǰ�����пո�                                       */
/*              ÿ������ַ���Ϊ LINE_MAX_LENGTH                          */
/*              ����������ַ���Ϊ NAME_MAX_LENGTH,�����������ò���       */
/*              ����ֵ����ַ���Ϊ VALUE_MAX_LENGTH,�������ض�            */
/**************************************************************************/
#include "getconfig.h"

/***************************************************************************/
/* �������ļ��ж�ȡ����һ��������Ӧ��ֵ                                    */
/* ������1,�����ļ�·��; 2,ƥ������; 3,����洢�ռ�                        */
/* ���أ�0,δ�ҵ�; 1,�ҵ��������Ƶ�ֵ                                      */
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
<<<<<<< HEAD
    while (fgets(config_linebuf, LINE_MAX_LENGTH, f) != NULL){
        if (strlen(config_linebuf) < 4){ //ȥ������ "=\r\n"
            continue;
        }

        if (config_linebuf[0] == '#'){//ȥ��ע���� "#"
=======
    while (fgets(config_linebuf, LINE_MAX_LENGTH, f) != NULL) {
        if (strlen(config_linebuf) < 4) { //去除空行 "=\r\n"
            continue;
        }

        if (config_linebuf[0] == '#') { //去除注释行 "#"
>>>>>>> 001198b671b63381741835d032c414b379b6614f
            continue;
        }

        if (config_linebuf[strlen(config_linebuf) - 1] == 10) {
            config_linebuf[strlen(config_linebuf) - 1] = '\0';
        }

        memset(line_name, 0, sizeof(line_name));
        leave_line = strstr(config_linebuf, config_sign);
<<<<<<< HEAD
        if (leave_line == NULL){//ȥ����"="�����
=======
        if (leave_line == NULL) { //去除无"="的情况
>>>>>>> 001198b671b63381741835d032c414b379b6614f
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
