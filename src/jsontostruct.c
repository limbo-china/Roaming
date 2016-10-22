#include "jsontostruct.h"
#define  SINGLEJSONLEN 150
RData_MsgContent* j2s(const char* _json)
{

    cJSON* json_rdata = cJSON_Parse(_json);

    s2j_create_struct_obj(struct_rdata, RData_MsgContent_T);

    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, roamprovince);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, region);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, usernumber);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, int, time);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, int, action);

    cJSON_Delete(json_rdata);

    

    char _roamprovince[3];
    char _region[7];
    strncpy(_roamprovince,(char *)struct_rdata->roamprovince,2);
    _roamprovince[2] = 0;
    strncpy(_region,(char *)struct_rdata->region,6);
    _region[6] = 0;

    RData_MsgContent* rdata = (RData_MsgContent *)malloc(sizeof(RData_MsgContent));

    rdata->length = 23;
    rdata->type = 3;
    rdata->roamprovince = (u_char )atoi(_roamprovince);
    rdata->region = (u_short)atoi(_region);
    strncpy(rdata->usernumber, (char*)struct_rdata->usernumber, 13);
    rdata->time = htonl(struct_rdata->time);
    rdata->action = struct_rdata->action;
    rdata->next = NULL;

    return rdata; 

    // char t_roamprovince[4];
    // char t_region[8];
    // char t_usernumber[15];
    // char t_time[15];
    // char t_action[3];

    // int i=0,j=0;

    // for(j=0;j<2;i++,j++)
    //  t_roamprovince[j] = *((u_char *)struct_rdata+i);
    // t_roamprovince[j] = 0;

    // for(j=0;j<5;i++,j++)
    //  t_region [j] = *((u_char *)struct_rdata+i);
    // t_region[j]=0;

    // for(j=0;j<13;i++,j++)
    //  t_usernumber [j] = *((u_char *)struct_rdata+i);
    // t_usernumber[j]=0;

    // for(j=0;j<12;i++,j++)
    //  t_time [j] = *((u_char *)struct_rdata+i);
    // t_time[j]=0;

    // for(j=0;j<1;i++,j++)
    //  t_action [j] = *((u_char *)struct_rdata+i);
    // t_action[j]=0;
}
void jsonStrParse(const char* jsonstr, hashtable_t* rdtable)
{
    int cur = 0, scur;
    char singleJson[SINGLEJSONLEN]= { 0 };

    RData_MsgContent* rdata;

    while (jsonstr[cur] != '[')
        cur++;
    while (jsonstr[cur] != ']') {
        if (jsonstr[cur] == '{') {
            scur = 0;
            while (jsonstr[cur] != '}') {
                singleJson[scur++] = jsonstr[cur];
                cur++;
            }
            singleJson[scur++] = jsonstr[cur];
            singleJson[scur] = 0;

            // got a single json string. transformat it and put into hashtable.
            rdata = j2s(singleJson);

            if (*rdata->usernumber != 0) { // data without a usernumber will not be inserted.
                printf("insert a record!\n");
                hashtable_insert(rdtable, rdata);
            }
            printf("hash count: %d\n", hashtable_count(rdtable));

            /////
            //free(rdata);
        }
        cur++;
    }
    //hashtable_trace(rdtable);
}