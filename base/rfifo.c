#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "rfifo.h"

uint32_t align_power(uint32_t num)
{
	uint32_t count, ret;

	count = 1;
	ret = num > 2147483648 ? 2147483648 : num;

	if(ret & (ret-1)){
		while(ret)
		{
			ret >>= 1;
			count <<= 1;
		}
		ret = count;
	}

	return ret;
}

rfifo_t *rfifo_create(size_t size, pthread_spinlock_t *lock)
{
	rfifo_t *fifo;
	size_t fifo_size = 0;

	fifo = (rfifo_t *)calloc(sizeof(rfifo_t), 1);
	if(fifo == NULL){
		perror("calloc");
		return NULL;
	}

	fifo_size = size < FIFOSIZE ? FIFOSIZE: size;

	if(fifo_size & (fifo_size-1)){
		fifo_size = align_power(fifo_size);
	}

	fifo->buff = (unsigned char *)calloc(sizeof(unsigned long), fifo_size);
	if(fifo->buff == NULL){
		//TODO:
		return NULL;
	}

	fifo->size = fifo_size * sizeof(unsigned long);
	fifo->in = fifo->out = 0;
	fifo->ncount = 0;
	fifo->lock = lock;

	return fifo;
}

uint32_t rfifo_count(rfifo_t *fifo)
{
	return fifo->ncount;
}

/**
 * @brief 向环形队列中添加数据
 * @param fifo	指向环形队列的指针
 * @param buff	添加的数据
 * @param len	添加的长度
 * @return 
 *		0		失败
 *		len		添加的长度
 */
int rfifo_put(rfifo_t *fifo, void *ptr)
{
	uint32_t l, len;
	unsigned long p;

	p = (unsigned long)ptr;
	len = sizeof(unsigned long);
	void *buff = (void *)(&p);

	if(len > (fifo->size - fifo->in + fifo->out))
		return -1;

	l = rfifo_min(len, fifo->size - (fifo->in & (fifo->size-1)));

	memcpy(fifo->buff+(fifo->in & (fifo->size-1)), buff, l);
	memcpy(fifo->buff, (unsigned char *)buff+l, len-l);

	fifo->in += len;

	atomic_add(fifo->ncount, 1);
	return 0;
}

/**
 * @brief 从环形队列中取数据
 * @param fifo	指向环形队列的指针
 * @param buff	获取的数据
 * @param len	获取的长度
 * @return 
 *		0		失败
 *		len		获取的长度
 */
void *rfifo_get(rfifo_t *fifo)
{
	uint32_t l, len;
	unsigned long p;

	len = sizeof(unsigned long);
	void *buff = (void *)(&p);

	if(len > (fifo->in - fifo->out))
		return NULL;

	l = rfifo_min(len, fifo->size - (fifo->out & (fifo->size-1)));

	memcpy(buff, fifo->buff+(fifo->out & (fifo->size-1)), l);
	memcpy((unsigned char *)buff+l, fifo->buff, len-l);

	fifo->out += len;

	atomic_sub(fifo->ncount, 1);
	return (void *)p;
}

/**
 * @brief 从环形队列中取数据
 * @param fifo	指向环形队列的指针
 * @param buff	获取的数据
 * @param len	获取的长度
 * @return 
 *		0		失败
 *		len		获取的长度
 */
void *rfifo_peek(rfifo_t *fifo)
{
	uint32_t l, len;
	unsigned long p;

	len = sizeof(unsigned long);
	void *buff = (void *)(&p);

	if(len > (fifo->in - fifo->out))
		return NULL;

	l = rfifo_min(len, fifo->size - (fifo->out & (fifo->size-1)));

	memcpy(buff, fifo->buff+(fifo->out & (fifo->size-1)), l);
	memcpy((unsigned char *)buff+l, fifo->buff, len-l);

	return (void *)p;
}

/**
 * @brief 销毁环形队列
 * @param fifo	指向环形队列的指针
 */
void rfifo_destroy(rfifo_t *fifo)
{
	if(fifo->buff)
		free(fifo->buff);

	if(fifo)
		free(fifo);
}
