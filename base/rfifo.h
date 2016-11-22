#ifndef _RFIFO_H_
#define _RFIFO_H_

#include <stdint.h>
#include <pthread.h>

#define rfifo_min(x,y) ((x) < (y) ? (x) : (y))
#define rfifo_max(x,y) ((x) < (y) ? (y) : (x))

#define FIFO_ERR_RFFULL		-1
#define FIFO_ERR_RFEMPTY	-2
#define FIFO_ERR_UNKNOWN	-3

#define FIFOSIZE 1024

#ifndef atomic_add
#define atomic_add(x, y) (__sync_add_and_fetch(&(x), y))
#endif
#ifndef atomic_sub
#define atomic_sub(x, y) (__sync_sub_and_fetch(&(x), y))
#endif

typedef struct rfifo_s rfifo_t;

struct rfifo_s
{
	size_t size;
	size_t nsize;
	unsigned char *buff;

	uint32_t in;
	uint32_t out;

	volatile uint32_t ncount;
	pthread_spinlock_t *lock;
};

/************************************************************/

rfifo_t *rfifo_create(size_t size, pthread_spinlock_t *lock);

uint32_t rfifo_count(rfifo_t *fifo);

/************************************************************/

int rfifo_put(rfifo_t *fifo, void *ptr);

void *rfifo_get(rfifo_t *fifo);

void *rfifo_peek(rfifo_t *fifo);

/************************************************************/

void rfifo_destroy(rfifo_t *fifo);

#endif /*_RFIFO_H_*/
