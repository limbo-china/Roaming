
#include "msgstruct.h"
#include "datahash.h"

const int usrNumLen = 13;

unsigned int rd_time33(char* data)
{
    unsigned int hash = 0;

    int i = 0;
    while (i < usrNumLen) {
        hash = (hash << 5) + hash + (*data++);
        i++;
    }

    return hash;
}

unsigned int rd_hash(void* n)
{
    RData_MsgContent* rd = (RData_MsgContent*)n;
    return (rd_time33((char*)rd->usernumber));
}
void rd_free(void* n)
{
    if (n)
        free(n);
}

int rd_compare(void* n1, void* n2)
{
    RData_MsgContent* r1 = (RData_MsgContent*)n1;
    RData_MsgContent* r2 = (RData_MsgContent*)n2;

    return (!strncmp(r1->usernumber, r2->usernumber,usrNumLen));
}