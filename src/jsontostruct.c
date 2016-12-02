    #include "jsontostruct.h"
#define SINGLEJSONLEN 150
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
    char _region1[3];
    char _region2[3];
    strncpy(_roamprovince, (char*)struct_rdata->roamprovince, 2);
    _roamprovince[2] = 0;
    strncpy(_region1, (char*)struct_rdata->region, 2);
    _region1[2] = 0;
    strncpy(_region2, (char*)struct_rdata->region+2,2);
    _region2[2] = 0;

    RData_MsgContent* rdata = (RData_MsgContent*)malloc(sizeof(RData_MsgContent));

    rdata->length = 22;
    rdata->type = 3;
    rdata->roamprovince = (u_char)atoi(_roamprovince);
    *((u_char *)&rdata->region) = (u_char)atoi(_region1);
    *((u_char *)&rdata->region+1) = (u_char)atoi(_region2);
    strncpy(rdata->usernumber, (char*)struct_rdata->usernumber+2, 11);
    rdata->usernumber[11] = '0';
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
void jsonStrParse(const char* jsonstr, int len,  hashtable_t* rdtable)
{
    int cur = 0, scur;
    char singleJson[SINGLEJSONLEN] = { 0 };

    RData_MsgContent* rdata;

    while (jsonstr[cur] != '{')
        cur++;
    //printf("1\n");
    while (cur <= len - 115) {
        if (jsonstr[cur] == '{' && jsonstr[cur+3] == 'o') {
            scur = 0;
            while (jsonstr[cur] != '}') {
                singleJson[scur++] = jsonstr[cur];
                cur++;
            }
            singleJson[scur++] = jsonstr[cur];
            singleJson[scur] = 0;
            // got a single json string. transformat it and put into hashtable.
            rdata = j2s(singleJson);

            pthread_mutex_lock(&send_mutex);
            if (*rdata->usernumber != 0) { // data without a usernumber will not be considered.

                // if(hashtable_search(rdtable,rdata) == NULL){
                //     printf("insert a record!\n");
                //     hashtable_insert(rdtable, rdata);
                //     sendRDataMsg(rdata, g_sockfd);
                // }
                // else{
                //     if(rdata->action == 0){
                //         hashtable_remove(rdtable,rdata);         
                //     }
                //     else{
                //         ////
                //         //...
                //         hashtable_remove(rdtable,rdata);  
                //         hashtable_insert(rdtable,rdata);
                //     }
                //     sendRDataMsg(rdata, g_sockfd);
                // }
                if(rdata->action == 0){
                    if(hashtable_search(rdtable,rdata) != NULL ){
                        printf("remove from table when leave.\n");
                        hashtable_remove(rdtable,rdata);
                    }
                    printf("send leave msg.\n");
                    sendRDataMsg(rdata, g_sockfd);
                }
                else{
                    RData_MsgContent* rdptr = (RData_MsgContent*)hashtable_search(rdtable,rdata);
                    if(rdptr == NULL){
                        printf("insert into table when enter.\n");
                        hashtable_insert(rdtable,rdata);
                    }
                    else{
                        printf("remove origin and send leave msg when enter.\n");
                        rdptr->action = 0;
                        sendRDataMsg(rdptr, g_sockfd);
                        hashtable_remove(rdtable,rdata);
                        printf("insert into table when enter.\n");
                        hashtable_insert(rdtable,rdata);
                    }
                    printf("send enter msg.\n");
                    sendRDataMsg(rdata,g_sockfd);
                }
            } 
            printf("hash count: %d\n", hashtable_count(rdtable));
            pthread_mutex_unlock(&send_mutex);
            sleep(1);
        }
        cur++;
    }
    //hashtable_trace(rdtable);
}