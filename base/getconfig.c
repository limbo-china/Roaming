/**************************************************************************/

/* ´ÓÅäÖÃÎÄ¼şÖĞ¶ÁÈ¡²ÎÊıÅäÖÃĞÅÏ¢                                           */
/* ÅäÖÃÎÄ¼ş¹æÔò£º                                                         */
/*              Ã¿ĞĞ½öÒ»¸ö²ÎÊıÖµ                                          */
/*              ĞĞÊ××Ö·ûÎª # ºÅµÄĞĞÎª×¢ÊÍĞĞ                               */
/*              Èç¹ûÒ»ĞĞÖĞ²»°üº¬ = ºÅ ÔòÄ¬ÈÏ¸ÃĞĞÎª×¢ÊÍĞĞ                  */
/*              ²ÎÊıÃû×ÖÓë²ÎÊıÖµÓÃ = ºÅÁ¬½Ó                               */
/*              ²ÎÊıÃû³ÆÖĞ²»ÄÜ°üº¬ = ºÅ                                   */
/*               = ºÅÇ°ºó²»ÄÜÓĞ¿Õ¸ñ                                       */
/*              Ã¿ĞĞ×î´ó×Ö·ûÊıÎª LINE_MAX_LENGTH                          */
/*              ²ÎÊıÃû×î´ó×Ö·ûÊıÎª NAME_MAX_LENGTH,³¬³¤½«¶ªÆú¸Ã²ÎÊı       */
/*              ²ÎÊıÖµ×î´ó×Ö·ûÊıÎª VALUE_MAX_LENGTH,³¬³¤½«½Ø¶Ï            */
/**************************************************************************/
#include "getconfig.h"

/***************************************************************************/
/* ´ÓÅäÖÃÎÄ¼şÖĞ¶ÁÈ¡µ¥¶ÀÒ»¸ö²ÎÊı¶ÔÓ¦µÄÖµ                                    */
/* ²ÎÊı£º1,ÅäÖÃÎÄ¼şÂ·¾¶; 2,Æ¥ÅäÃû³Æ; 3,Êä³ö´æ´¢¿Õ¼ä                        */
/* ·µ»Ø£º0,Î´ÕÒµ½; 1,ÕÒµ½·ûºÏÃû³ÆµÄÖµ                                      */
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
        if (strlen(config_linebuf) < 4){ //È¥³ı¿ÕĞĞ "=\r\n"
            continue;
        }

        if (config_linebuf[0] == '#'){//È¥³ı×¢ÊÍĞĞ "#"
=======
    while (fgets(config_linebuf, LINE_MAX_LENGTH, f) != NULL) {
        if (strlen(config_linebuf) < 4) { //å»é™¤ç©ºè¡Œ "=\r\n"
            continue;
        }

        if (config_linebuf[0] == '#') { //å»é™¤æ³¨é‡Šè¡Œ "#"
>>>>>>> 001198b671b63381741835d032c414b379b6614f
            continue;
        }

        if (config_linebuf[strlen(config_linebuf) - 1] == 10) {
            config_linebuf[strlen(config_linebuf) - 1] = '\0';
        }

        memset(line_name, 0, sizeof(line_name));
        leave_line = strstr(config_linebuf, config_sign);
<<<<<<< HEAD
        if (leave_line == NULL){//È¥³ıÎŞ"="µÄÇé¿ö
=======
        if (leave_line == NULL) { //å»é™¤æ— "="çš„æƒ…å†µ
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
