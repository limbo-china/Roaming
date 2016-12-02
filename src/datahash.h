#ifndef DATAHASH_H_
#define DATAHASH_H_

unsigned int rd_time33(char *data);
unsigned int rd_hash(void *n);
void rd_free(void *n);
int rd_compare(void *n1, void *n2);

#endif