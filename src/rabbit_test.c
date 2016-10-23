#include "rabbit_test.h"
#include <stdio.h>

char jsontest[40000];

void getjson()
{
    FILE* f;
    if ((f = fopen(TESTFILE_PATH, "r+")) == NULL)
        printf("cannot open file.\n");

    int i = 0;
    char c;
    while ((c = fgetc(f)) != EOF) {
        jsontest[i++] = c;
    }

    jsontest[i] = 0;
}