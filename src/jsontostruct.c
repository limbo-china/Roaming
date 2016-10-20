#include "jsontostruct.h"

RData_MsgContent* j2s(const char* _json)
{

    cJSON* json_rdata = cJSON_Parse(_json);

    s2j_create_struct_obj(struct_rdata, RData_MsgContent_T);

    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, roamprovince);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, region);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, usernumber);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, time);
    s2j_struct_get_basic_element(struct_rdata, json_rdata, string, action);

    cJSON_Delete(json_rdata);

    RData_MsgContent* rdata = malloc(sizeof(RData_MsgContent));

    strncpy(rdata->length, "18", 2);
    strncpy(rdata->type, "3", 1);
    strncpy(rdata->roamprovince, (char*)struct_rdata, sizeof(RData_MsgContent_T));

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