/*
 # hashtable.c
 # version:3.1
 [2.1]improvement----Do NOT use private hashnode_t to maintain the objs' link
 [3.0]improvement----add hashtable_search_traverse() interface
 [3.1]improvement----add hashtable_remove_byaddr() interface
 ----add time_out function
 ----add table_lock function
 IMPORTATN: time_out and table_lock function should NOT use simultaneously!
 #author:zyz
 #author:typ
 #date:2013-08-18
 */

#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 Credit for primes table: Aaron Krowne
 http://br.endernet.org/~akrowne/
 http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
 */
static const unsigned int primes[] = { 53, 97, 193, 389, 769, 1543, 3079, 6151,
		12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869, 3145739,
		6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189,
		805306457, 1610612741 };
const unsigned int prime_table_length = sizeof(primes) / sizeof(primes[0]);
const float max_load_factor = 0.90;

#ifdef HASHTABLE_TABLE_LOCK
static int hashatble_tablelock_init(pthread_mutex_t *pmutex)
{
	int ret;

	pthread_mutexattr_t attr;
	ret = pthread_mutexattr_init(&attr);
	if (ret)
	{
		printf("Creating a tablelock mutex is failed.\n");
		pthread_mutex_destroy(pmutex);
		return 0;
	}
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP);
	ret = pthread_mutex_init(pmutex, &attr);
	if (ret)
	{
		printf("Init a tablelock mutex is failed.\n");
		pthread_mutex_destroy(pmutex);
		return 0;
	}

	return 1;
}
#endif

/*****************************************************************************/
hashtable_t *
hashtable_create(unsigned int minsize, unsigned int struct_size,
		unsigned int timeout_time, unsigned int table_lock,
		void (*free_value)(void *), unsigned int (*hashfunc)(void*),
		int (*compfunc)(void*, void*))
{
	hashtable_t *h;
	unsigned int pindex, size;
	int ptr_len;

	size = primes[0];
	ptr_len = sizeof(void *);

	if (!free_value || !hashfunc || !compfunc)
		return NULL;
	if (struct_size <= ptr_len)
	{
		return NULL;
	}
	/* Check requested hashtable isn't too large */
	if (minsize > (1u << 30))
		return NULL;
	/* Enforce size as prime */
	for (pindex = 0; pindex < prime_table_length; pindex++)
	{
		if (primes[pindex] > minsize)
		{
			size = primes[pindex];
			break;
		}
	}
	h = (hashtable_t *) malloc(sizeof(hashtable_t));
	if (NULL == h)
		return NULL;
	h->table = (void*) malloc(ptr_len * size);
	memset(h->table, 0, size * ptr_len);
	h->tablelength = size;
	h->primeindex = pindex;
	h->nodecount = 0;
	h->offset = struct_size - ptr_len;		//modified by zyz
	h->free_value = free_value;
	h->hashfunc = hashfunc;
	h->compfunc = compfunc;
	h->loadlimit = size * max_load_factor + 1;

#ifdef HASHTABLE_TIMEOUT
	h->offset_timeout_pre = struct_size - 2 * ptr_len;
	h->offset_timeout_next = struct_size - 3 * ptr_len;
	h->offset_time = struct_size - 4 * ptr_len;

	h->timeout_head = (void *) calloc(struct_size, 1);
	if (!h->timeout_head)
		return NULL;
	h->timeout_time = timeout_time;
	h->timeout_tail = h->timeout_head;
	h->struct_size = struct_size;
	h->head_time = 0;
	h->tail_time = 0;
#endif

#ifdef HASHTABLE_TABLE_LOCK
	int i = 0;
	h->table_lock = table_lock;
	if (h->table_lock)
	{
		h->tablelock_mutex = (pthread_mutex_t *) malloc(
				h->tablelength * sizeof(pthread_mutex_t));
		memset(h->tablelock_mutex, 0, h->tablelength * sizeof(pthread_mutex_t));
		for (i = 0; i < h->tablelength; i++)
		{
			if (!hashatble_tablelock_init(&h->tablelock_mutex[i]))
				return NULL;
		}
	}
#endif

	return h;
}

/*****************************************************************************/
void hashtable_init(hashtable_t *h)
{
	int i;
	void *e, *f;
	if (!h)
		return;
	for (i = 0; i < h->tablelength; i++)
	{
		e = h->table[i];
		while (NULL != e)
		{
			f = e;
			e = (void *) *(unsigned long *) (e + h->offset);
			h->free_value(f);
		}
	}
	memset(h->table, 0, h->tablelength * sizeof(void *));
	h->nodecount = 0;

#ifdef HASHTABLE_TIMEOUT
	if (h->timeout_head)
		free(h->timeout_head);
	h->timeout_head = (void *) calloc(h->struct_size, 1);
	if (!h->timeout_head)
		return;
	h->timeout_tail = h->timeout_head;
	h->head_time = 0;
	h->tail_time = 0;
#endif
}

/*****************************************************************************/
unsigned int hash(hashtable_t *h, void *v)
{
	return h->hashfunc(v);
}

/*****************************************************************************/
/* indexfor */
static inline unsigned int indexfor(unsigned int tablelength,
		unsigned int hashvalue)
{
	return (hashvalue % tablelength);
}
;

/*****************************************************************************/
static int hashtable_expand(hashtable_t *h)
{
	void *e, **newtable;
	unsigned int newsize, i, index;

	if (!h)
		return 0;
	/* Double the size of the table to accomodate more entries */
	/* Check we're not hitting max capacity */
	if (h->primeindex == (prime_table_length - 1))
		return 0;
	newsize = primes[++(h->primeindex)];

	newtable = (void **) malloc(sizeof(void*) * newsize);
	if (NULL != newtable)
	{
		memset(newtable, 0, newsize * sizeof(void *));
		/* This algorithm is not 'stable'. ie. it reverses the list
		 * when it transfers entries between the tables */
		for (i = 0; i < h->tablelength; i++)
		{
			while (NULL != (e = h->table[i]))
			{
				h->table[i] = (void *) *(unsigned long *) (e + h->offset);
				index = indexfor(newsize, hash(h, e));
				//memcpy(e+h->offset, &(newtable[index]), sizeof(void *));
				*(unsigned long *) (e + h->offset) =
						(unsigned long) newtable[index];
				newtable[index] = e;
			}
		}
		free(h->table);
		h->table = newtable;

#ifdef HASHTABLE_TABLE_LOCK
		if (h->table_lock)
		{
			free(h->tablelock_mutex);

			h->tablelock_mutex = (pthread_mutex_t *) malloc(
					newsize * sizeof(pthread_mutex_t));
			memset(h->tablelock_mutex, 0, newsize * sizeof(pthread_mutex_t));
			for (i = 0; i < newsize; i++)
			{
				if (!hashatble_tablelock_init(&h->tablelock_mutex[i]))
					return 0;
			}
		}
#endif
	}
	h->tablelength = newsize;
	h->loadlimit = newsize * max_load_factor + 1;
	return -1;
}

/*****************************************************************************/
unsigned int hashtable_count(hashtable_t *h)
{
	if (!h)
		return 0;
	return h->nodecount;
}

#ifdef HASHTABLE_TIMEOUT
static int insert_timeout_element(hashtable_t *h, void* element)
{
	if (!h || !element)
		return -1;
	set_element_timeout_next(h->timeout_tail, element, h);
	set_element_timeout_pre(element, h->timeout_tail, h);
	h->timeout_tail = element;
	set_element_timeout_next(element, 0, h);
	return 0;
}
static int delete_timeout_element(hashtable_t *h, void* element)
{
	void *element_pre;
	void *element_next;
	if (!h || !element)
		return -1;
	if (element == h->timeout_head)
		return -1;
	if (element != h->timeout_tail)
	{
		element_pre = get_element_pre_ptr(element, h);
		element_next = get_element_next_ptr(element, h);
		set_element_timeout_pre(element_next, element_pre, h);
		set_element_timeout_next(element_pre, element_next, h);
	} else
	{
		element_pre = get_element_pre_ptr(element, h);
		set_element_timeout_next(element_pre, 0, h);
		h->timeout_tail = element_pre;
	}
	return 0;
}
static int update_timeout_element(hashtable_t *h, void* element)
{
	if (!h || !element)
		return -1;
	if (element == h->timeout_head)
		return -1;
	if (element == h->timeout_tail)
	{
		return 0;
	}

	void *element_pre;
	void *element_next;

	element_pre = get_element_pre_ptr(element, h);
	element_next = get_element_next_ptr(element, h);
	set_element_timeout_pre(element_next, element_pre, h);
	set_element_timeout_next(element_pre, element_next, h);
	*(unsigned long *) (element + h->offset_time) = time(NULL);
	return insert_timeout_element(h, element);
}

static int hashtable_delete_one_timeout_element(hashtable_t *h,
		time_t base_time)
{
	if (!h || h->timeout_head == h->timeout_tail)
		return 0;
	void *head_next;
	head_next = get_element_next_ptr(h->timeout_head, h);
	if (head_next == NULL)
		return 0;
	unsigned int gap = base_time
			- *(unsigned long *) (head_next + h->offset_time);

	if (gap < h->timeout_time)
		return 0;

	hashtable_remove_byaddr(h, head_next);
	return -1;
}

static void hashtable_delete_all_timeout_element(hashtable_t *h, time_t now)
{
	int ret;
	while (1)
	{
		ret = hashtable_delete_one_timeout_element(h, now);
		if (!ret)
			break;
	}
}

#endif

/*****************************************************************************/
int hashtable_insert(hashtable_t *h, void *v)
{
	unsigned int index;

	if (!h)
		return 0;
#ifdef HASHTABLE_TABLE_LOCK
	if (!h->table_lock)
	{
#endif
		/* This method allows duplicate keys - but they shouldn't be used */
		if (++(h->nodecount) > h->loadlimit)
		{
			/* Ignore the return value. If expand fails, we should
			 * still try cramming just this value into the existing table
			 * -- we may not have memory for a larger table, but one more
			 * element may be ok. Next time we insert, we'll try expanding again.*/
			hashtable_expand(h);
		}
#ifdef HASHTABLE_TABLE_LOCK
	} else
		__sync_fetch_and_add(&h->nodecount, 1);
#endif

	index = indexfor(h->tablelength, hash(h, v));

#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_lock(&h->tablelock_mutex[index]);
#endif

	*(unsigned long *) (v + h->offset) = (unsigned long) h->table[index];
	h->table[index] = v;

#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_unlock(&h->tablelock_mutex[index]);
#endif

#ifdef HASHTABLE_TIMEOUT
	if (h->timeout_time)
	{
		*(unsigned long *) (v + h->offset_time) = time(NULL);
		insert_timeout_element(h, v);
		hashtable_delete_all_timeout_element(h, time(NULL));
	}
#endif
	return -1;
}

/*****************************************************************************/
void * /* returns value associated with key */
hashtable_search(hashtable_t *h, void *v)
{
	void *e;
	unsigned int hashvalue, index;

	if (!h)
		return NULL;

	hashvalue = hash(h, v);
	index = indexfor(h->tablelength, hashvalue);
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_lock(&h->tablelock_mutex[index]);
#endif
	e = h->table[index];
	while (NULL != e)
	{
		/* Check hash value to short circuit heavier comparison */
		if (h->compfunc(v, e))
		{
#ifdef HASHTABLE_TIMEOUT
			if (h->timeout_time)
			{
				update_timeout_element(h, e);
			}
#endif
#ifdef HASHTABLE_TABLE_LOCK
			if (h->table_lock)
				pthread_mutex_unlock(&h->tablelock_mutex[index]);
#endif
			return e;
		}
		e = (void *) *(unsigned long *) (e + h->offset);
	}
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_unlock(&h->tablelock_mutex[index]);
#endif
	return NULL;
}

/*****************************************************************************/
void /* every objs hitted with recall callback func */
hashtable_search_traverse(hashtable_t *h, void *v, void (*cb)(void *, void *),
		void * cb_obj)
{
	void *e;
	unsigned int hashvalue, index;

	if (!h)
		return;

	hashvalue = hash(h, v);
	index = indexfor(h->tablelength, hashvalue);
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_lock(&h->tablelock_mutex[index]);
#endif
	e = h->table[index];
	while (NULL != e)
	{
		/* Check hash value to short circuit heavier comparison */
		if (h->compfunc(v, e))
			cb(e, cb_obj);
		e = (void *) *(unsigned long *) (e + h->offset);
	}
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_unlock(&h->tablelock_mutex[index]);
#endif
	return;
}

void * /* returns value associated with key */
hashtable_search_tablelock(hashtable_t *h, void *v, unsigned int *table)
{
	void *e;
	unsigned int hashvalue, index;

	if (!h)
		return NULL;

	hashvalue = hash(h, v);
	index = indexfor(h->tablelength, hashvalue);
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_lock(&h->tablelock_mutex[index]);
#endif
	e = h->table[index];
	while (NULL != e)
	{
		/* Check hash value to short circuit heavier comparison */
		if (h->compfunc(v, e))
		{
#ifdef HASHTABLE_TIMEOUT
			if (h->timeout_time)
			{
				update_timeout_element(h, e);
			}
#endif
			*table = index;
			return e;
		}
		e = (void *) *(unsigned long *) (e + h->offset);
	}
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_unlock(&h->tablelock_mutex[index]);
#endif
	return NULL;
}

/*****************************************************************************/
int /* returns 1 success || 0 error */
hashtable_remove(hashtable_t *h, void *v)
{
	void *e, **pE;

	if (!h)
		return 0;
	/* TODO: consider compacting the table when the load factor drops enough,
	 *       or provide a 'compact' method. */
	unsigned int hashvalue, index;

	hashvalue = hash(h, v);
	index = indexfor(h->tablelength, hashvalue);
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_lock(&h->tablelock_mutex[index]);
#endif
	pE = &(h->table[index]);
	e = *pE;
	while (NULL != e)
	{
		/* Check hash value to short circuit heavier comparison */
		if (h->compfunc(v, e))
		{
			*pE = (void *) *(unsigned long *) (e + h->offset);
#ifdef HASHTABLE_TIMEOUT
			if (h->timeout_time)
			{
				delete_timeout_element(h, e);
			}
#endif
			h->free_value(e);
#ifdef HASHTABLE_TABLE_LOCK
			if (h->table_lock)
				pthread_mutex_unlock(&h->tablelock_mutex[index]);
			__sync_fetch_and_sub(&h->nodecount, 1);
#else
			h->nodecount--;
#endif
			return 1;
		}
		pE = e + h->offset;
		e = (void *) *(unsigned long *) (e + h->offset);
	}
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_unlock(&h->tablelock_mutex[index]);
#endif
	return 0;
}

/*****************************************************************************/
int /* returns 1 success || 0 error */
hashtable_remove_notablelock(hashtable_t *h, void *v)
{
	void *e, **pE;

	if (!h)
		return 0;
	/* TODO: consider compacting the table when the load factor drops enough,
	 *       or provide a 'compact' method. */
	unsigned int hashvalue, index;

	hashvalue = hash(h, v);
	index = indexfor(h->tablelength, hashvalue);
	pE = &(h->table[index]);
	e = *pE;
	while (NULL != e)
	{
		/* Check hash value to short circuit heavier comparison */
		if (h->compfunc(v, e))
		{
			*pE = (void *) *(unsigned long *) (e + h->offset);
			//h->nodecount--;
			__sync_fetch_and_sub(&h->nodecount, 1);
#ifdef HASHTABLE_TIMEOUT
			if (h->timeout_time)
			{
				delete_timeout_element(h, e);
			}
#endif
			h->free_value(e);
			return 1;
		}
		pE = e + h->offset;
		e = (void *) *(unsigned long *) (e + h->offset);
	}
	return 0;
}

/*****************************************************************************/
int /* returns 1 success || 0 error */
hashtable_remove_byaddr(hashtable_t *h, void *v)
{
	void *e, **pE;

	if (!h)
		return 0;
	/* TODO: consider compacting the table when the load factor drops enough,
	 *       or provide a 'compact' method. */
	unsigned int hashvalue, index;

	hashvalue = hash(h, v);
	index = indexfor(h->tablelength, hashvalue);
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_lock(&h->tablelock_mutex[index]);
#endif
	pE = &(h->table[index]);
	e = *pE;
	while (NULL != e)
	{
		if (v == e)
		{
			*pE = (void *) *(unsigned long *) (e + h->offset);
#ifdef HASHTABLE_TIMEOUT
			if (h->timeout_time)
			{
				delete_timeout_element(h, e);
			}
#endif
			h->free_value(e);
#ifdef HASHTABLE_TABLE_LOCK
			if (h->table_lock)
				pthread_mutex_unlock(&h->tablelock_mutex[index]);
			__sync_fetch_and_sub(&h->nodecount, 1);
#else
			h->nodecount--;
#endif
			return 1;
		}
		pE = e + h->offset;
		e = (void *) *(unsigned long *) (e + h->offset);
	}
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		pthread_mutex_unlock(&h->tablelock_mutex[index]);
#endif
	return 0;
}

/*****************************************************************************/
/* destroy */
void hashtable_free(hashtable_t *h)
{
	unsigned int i;
	void *e, *f, **table;
	table = h->table;

	if (!h)
		return;

	for (i = 0; i < h->tablelength; i++)
	{
		e = table[i];
		while (NULL != e)
		{
			f = e;
			e = (void *) *(unsigned long *) (e + h->offset);
			h->free_value(f);
		}
	}
#ifdef HASHTABLE_TIMEOUT
	free(h->timeout_head);
#endif
#ifdef HASHTABLE_TABLE_LOCK
	if (h->table_lock)
		free(h->tablelock_mutex);
#endif
	free(h->table);
	free(h);
}

/******************************************************************************/
/* hashtable_trace
 * function to output the values in hashtable by turn
 */
void hashtable_trace(hashtable_t *h)
{
	int i;
	void *e;

	if (!h)
		return;

	printf("/******************Hashtable_trace********************/\n\n");
	printf(
			"hashtable->nodecount : %u\nhashtable->loadlimit : %u\nhashtable->tablelength : %u\n\n",
			h->nodecount, h->loadlimit, h->tablelength);
	printf("====================table cotents====================\n");

	for (i = 0; i < h->tablelength; i++)
	{
		e = h->table[i];
		printf("table[%d] :\n", i);
		while (NULL != e)
		{
			printf("\tentry->value :%p\n", e);
			e = (void *) *(unsigned long *) (e + h->offset);
		}
	}
	return;
}

/*****************************************************************************/
int hashtable_insert_notablelock(hashtable_t *h, void *v)
{
	unsigned int index;

	if (!h)
		return 0;

	/* This method allows duplicate keys - but they shouldn't be used */
	if (++(h->nodecount) > h->loadlimit)
	{
		/* Ignore the return value. If expand fails, we should
		 * still try cramming just this value into the existing table
		 * -- we may not have memory for a larger table, but one more
		 * element may be ok. Next time we insert, we'll try expanding again.*/
		hashtable_expand(h);
	}

	__sync_fetch_and_add(&h->nodecount, 1);

	index = indexfor(h->tablelength, hash(h, v));

	*(unsigned long *) (v + h->offset) = (unsigned long) h->table[index];
	h->table[index] = v;

#ifdef HASHTABLE_TIMEOUT
	if (h->timeout_time)
	{
		*(unsigned long *) (v + h->offset_time) = time(NULL);
		insert_timeout_element(h, v);
		hashtable_delete_all_timeout_element(h, time(NULL));
	}
#endif
	return -1;
}

