/*
 # hashtable.c
 # version:3.1
 [2.1]improvement----Do NOT use private hashnode_t to maintain the objs' link
 [3.0]improvement----add hashtable_search_traverse() interface
 [3.1]improvement----add hashtable_remove_byaddr() interface
 ----add time_out function
 ----add table_lock function
 IMPORTATN: time_out and table_lock function should NOT use simultaneously!
 #author:typ
 #date:2013-08-18
 */

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <time.h>

//#define HASHTABLE_TIMEOUT
#define HASHTABLE_TABLE_LOCK
#ifdef HASHTABLE_TABLE_LOCK
#include <pthread.h>
#endif

#define get_element_pre_ptr(element,h) (void *)*(long *)((char *)element+h->offset_timeout_pre)
#define get_element_next_ptr(element,h) (void *)*(long *)((char *)element+h->offset_timeout_next)
#define set_element_timeout_pre(element,pre_ptr,h) *(unsigned long *)((char *)element+h->offset_timeout_pre)=(unsigned long)pre_ptr
#define set_element_timeout_next(element,next_ptr,h) *(unsigned long *)((char *)element+h->offset_timeout_next)=(unsigned long)next_ptr

#define get_time_of_element_ptr(element, h)		(unsigned long *)((char *)element+h->offset_time)

/*****************************************************************************/
typedef struct hashtable
{
	unsigned int tablelength;
	void **table;
	unsigned int nodecount;
	unsigned int loadlimit;
	unsigned int primeindex;
	unsigned int offset;			//the offset of next obj in bucket link
	void (*free_value)(void *vl);
	unsigned int (*hashfunc)(void *k);
	int (*compfunc)(void *k1, void *k2);

#ifdef HASHTABLE_TIMEOUT
	unsigned int offset_time;
	void *timeout_head;
	void *timeout_tail;
	time_t head_time;
	time_t tail_time;
	unsigned int struct_size;
	unsigned int timeout_time;
	unsigned int offset_timeout_pre;
	unsigned int offset_timeout_next;
#endif

#ifdef HASHTABLE_TABLE_LOCK
	unsigned int table_lock;
	pthread_mutex_t *tablelock_mutex;
#endif

} hashtable_t;

/*****************************************************************************
 * hashtable_create

 * @name                    hashtable_create
 * @param   minsize         minimum initial size of hashtable
 * @param   struct_size 	   size of obj in hashtable
 * @param   free_value      structure to stroe functions of free_key and free_value
 * @param   hashfunc        function for hashing keys
 * @param   compfunc        function for determining key equality
 * @return                  newly created hashtable or NULL on failure
 */

hashtable_t *
hashtable_create(unsigned int minsize, unsigned int struct_size,
		unsigned int timeout_time, unsigned int table_lock,
		void (*free_value)(void *), unsigned int (*hashfunc)(void*),
		int (*compfunc)(void*, void*));

/*****************************************************************************
 * hashtable_init  
 
 * @func                    init the hashtable, free the values but not the table   
 * @name                    hashtable_init
 * @param   h               the hashtable
 */

void
hashtable_init(hashtable_t *h);

/*****************************************************************************
 * hashtable_insert

 * @name        hashtable_insert
 * @param   h   the hashtable to insert into
 * @param   v   the value to search for  - indeed delete by key(call getkey func)
 * @return      non-zero for successful insertion
 *
 * This function will cause the table to expand if the insertion would take
 * the ratio of entries to table size over the maximum load factor.
 *
 * This function does not check for repeated insertions with a duplicate key.
 * The value returned when using a duplicate key is undefined -- when
 * the hashtable changes size, the order of retrieval of duplicate key
 * entries is reversed.
 * If in doubt, remove before insert.
 */

int
hashtable_insert(hashtable_t *h, void *v);

/*****************************************************************************
 * hashtable_search

 * @name        hashtable_search
 * @param   h   the hashtable to search
 * @param   v	the value to search for  - indeed delete by key(call getkey func)
 * @return      the value associated with the key, or NULL if none found
 */

void *
hashtable_search(hashtable_t *h, void *v);

/*****************************************************************************
 * hashtable_search

 * @name        hashtable_search_traverse
 * @param   h   the hashtable to search
 * @param   v	  the value to search for  - indeed delete by key(call getkey func)
 * @param   cb every objs hitted with recall cb() func 
 * @param   cb_obj  cb_obj will be send back as the 2nd param
 * @return     
 */
void
hashtable_search_traverse(hashtable_t *h, void *v, void (*cb)(void *, void *),
		void * cb_obj);

/*****************************************************************************
 * hashtable_search

 * @name        hashtable_search_tablelock
 * @param   h   the hashtable to search
 * @param   v	  the value to search for  - indeed delete by key(call getkey func)
 * @param   table  the table index that be locked, user MUST unlock this table later!
 * @return     
 */
void * /* returns value associated with key */
hashtable_search_tablelock(hashtable_t *h, void *v, unsigned int *table);

/*****************************************************************************
 * hashtable_remove

 * @name        hashtable_remove
 * @param   h   the hashtable to remove the item from
 * @param   v   the value to search for  - indeed delete by key(call getkey func)
 * @return      1 success || 0 fail
 */

int /* returns value */
hashtable_remove(hashtable_t *h, void *v);

/*****************************************************************************
 * hashtable_remove_byaddr

 * @name        hashtable_remove_byaddr
 * @param   h   the hashtable to remove the item from
 * @param   v   the value to search for  - indeed delete by addr
 * @return      1 success || 0 fail
 */

int /* returns value */
hashtable_remove_byaddr(hashtable_t *h, void *v);

/*****************************************************************************
 * hashtable_remove_notablelock

 * @name        hashtable_remove_notablelock, this operation will NOT use tablelock
 * @param   h   the hashtable to remove the item from
 * @param   v   the value to search for  - indeed delete by key(call getkey func)
 * @return      1 success || 0 fail
 */
int /* returns 1 success || 0 error */
hashtable_remove_notablelock(hashtable_t *h, void *v);

/*****************************************************************************
 * hashtable_count

 * @name        hashtable_count
 * @param   h   the hashtable
 * @return      the number of items stored in the hashtable
 */
unsigned int
hashtable_count(hashtable_t *h);

/*****************************************************************************
 * hashtable_free

 * @name        hashtable_free
 * @param   h   the hashtable
 */

void
hashtable_free(hashtable_t *h);

/*****************************************************************************
 * hashtable_trace
 * function to output the values in hashtable by turn
 * 
 * @name		hashtable_trace
 * @param	h   the hashtable
 */
void
hashtable_trace(hashtable_t *h);

int hashtable_insert_notablelock(hashtable_t *h, void *v);
#endif /* __HASHTABLE_H__ */

