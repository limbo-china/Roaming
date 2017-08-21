#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define DUMP_FILE_PATH "./hashtable_new.dmp"

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long long u_longlong;

typedef struct R_DATA_MEG_CONTENT {
    u_char length;
    u_char type;
    u_char roamprovince;
    u_short region;
    char usernumber[12];
    u_int time;
    u_char action;

    void* next;
} RData_MsgContent;

void exmdata(int num, RData_MsgContent* t);

int main(){

    FILE* f;
    f = fopen(DUMP_FILE_PATH, "r");
    int readNum =0;

    if (f == NULL) {
        printf("Open Dumpfile failed\n");
        return 0;
    }

    printf("Reading the dumpfile.\n");

    while(1){

        RData_MsgContent* t = (RData_MsgContent*)malloc(sizeof(RData_MsgContent));

        if(fread(t, sizeof(RData_MsgContent), 1, f) != 1)
            break;

        exmdata(readNum,t);

        readNum = readNum +1;
   
    }
    printf("Read done. %d records got.\n",readNum);
}

void exmdata(int num, RData_MsgContent* t)
{

    if((strcmp(t->usernumber,"18019167479")==0)||(strcmp(t->usernumber,"18019167492")==0))
    {
        printf("%d %s\n",num,t->usernumber);
    }
}