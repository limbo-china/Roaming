/*
 	# hashtable.c
 	# version:2.1
 			improvement----Do NOT use private hashnode_t to maintain the objs' link
 	#author:typ
 	#date:2009-09-14
*/

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

/*****************************************************************************/
typedef struct hashtable {
    unsigned int tablelength;
    void **table;
    unsigned int nodecount;
    unsigned int loadlimit;
    unsigned int primeindex;
    unsigned int offset;			//the offset of next obj in bucket link
    void (*free_value)(void *vl);
    unsigned int (*hashfunc) (void *k);
    int (*compfunc) (void *k1, void *k2);
}hashtable_t;


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
hashtable_create(unsigned int minsize,
				      unsigned int struct_size,
				 void (*free_value)(void *),
                 unsigned int (*hashfunc) (void*),
                 int (*compfunc) (void*,void*));
                 
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
hashtable_insert(hashtable_t *h,void *v);


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
 * hashtable_remove
   
 * @name        hashtable_remove
 * @param   h   the hashtable to remove the item from
 * @param   v   the value to search for  - indeed delete by key(call getkey func)
 * @return      1 success || 0 fail
 */

int  /* returns value */
hashtable_remove(hashtable_t *h, void *v);


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

#endif /* __HASHTABLE_H__ */


