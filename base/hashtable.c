/*
 	# hashtable.c
 	# version:2.1
 			improvement----Do NOT use private hashnode_t to maintain the objs' link
 	#author:typ
 	#date:2009-09-14
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
static const unsigned int primes[] = {
53, 97, 193, 389,
769, 1543, 3079, 6151,
12289, 24593, 49157, 98317,
196613, 393241, 786433, 1572869,
3145739, 6291469, 12582917, 25165843,
50331653, 100663319, 201326611, 402653189,
805306457, 1610612741
};
const unsigned int prime_table_length = sizeof(primes)/sizeof(primes[0]);
const float max_load_factor = 0.90;

/*****************************************************************************/
hashtable_t *
hashtable_create(unsigned int minsize,
				      unsigned int struct_size,
				void (*free_value)(void *),
                 unsigned int (*hashfunc) (void*),
                 int (*compfunc) (void*,void*))
{
	hashtable_t *h;
    unsigned int pindex, size;
    int ptr_len;	
	
    size = primes[0];
    ptr_len = sizeof(void *);
	
	
	if(!free_value||!hashfunc||!compfunc)
    	return NULL;
	if(struct_size<=ptr_len)
	{
		//printf("Struct size is too small..\n");
		return NULL;
	}
    /* Check requested hashtable isn't too large */
    if (minsize > (1u << 30)) return NULL;
    /* Enforce size as prime */
    for (pindex=0; pindex < prime_table_length; pindex++) {
        if (primes[pindex] > minsize) { size = primes[pindex]; break; }
    }
    h = (hashtable_t *)malloc(sizeof(hashtable_t));
    if (NULL == h) return NULL; 
    h->table =(void*)malloc(ptr_len* size);
    memset(h->table, 0, size * ptr_len);
    h->tablelength  = size;
    h->primeindex   = pindex;
    h->nodecount   = 0;
	h->offset = struct_size - ptr_len;		//modified by zyz
    h->free_value = free_value;
    h->hashfunc     = hashfunc;
    h->compfunc     = compfunc;
    h->loadlimit    = size * max_load_factor+1;

    return h;
}

/*****************************************************************************/
void
hashtable_init(hashtable_t *h)
{
	int i;
	void *e, *f;
	if(!h)
		return;
	for (i = 0; i < h->tablelength; i++)
	{
	        e = h->table[i];
	        while (NULL != e)
	        { 
	        	f = e; 
			e = (void *)*(unsigned long *)(e+h->offset);
			h->free_value(f); 
		}
	}
    memset(h->table, 0, h->tablelength * sizeof(void *));
	h->nodecount = 0;
}


/*****************************************************************************/
unsigned int
hash(hashtable_t *h, void *v)
{
    /* Aim to protect against poor hash functions by adding logic here
     * - logic taken from java 1.4 hashtable source */
    /*unsigned int i = h->hashfunc(v);
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); 
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); 
    return i;*/
    return h->hashfunc(v);
}

/*****************************************************************************/
/* indexfor */
static inline unsigned int
indexfor(unsigned int tablelength, unsigned int hashvalue) {
    return (hashvalue % tablelength);
};

/*****************************************************************************/
#if 0
static int
hashtable_expand(hashtable_t *h)
{
	void *e, **newtable;
    unsigned int newsize, i, index;
    
	if(!h)
		return 0;
    /* Double the size of the table to accomodate more entries */
    /* Check we're not hitting max capacity */
    if (h->primeindex == (prime_table_length - 1)) return 0;
    newsize = primes[++(h->primeindex)];

    newtable = (void **)malloc(sizeof(void*) * newsize);
    if (NULL != newtable)
    {
        memset(newtable, 0, newsize * sizeof(void *));
        /* This algorithm is not 'stable'. ie. it reverses the list
         * when it transfers entries between the tables */
        for (i = 0; i < h->tablelength; i++) {
            while (NULL != (e = h->table[i])) {
                h->table[i] =  (void *)*(unsigned long *)(e+h->offset);
                index = indexfor(newsize,hash(h,e));
                //memcpy(e+h->offset, &(newtable[index]), sizeof(void *));
                 *(unsigned long *)(e+h->offset)  = (unsigned long )newtable[index];
                newtable[index] = e;
            }
        }
        free(h->table);
        h->table = newtable;
    }
    h->tablelength = newsize;
    h->loadlimit   = newsize * max_load_factor+1;
    return -1;
}
#endif

/*****************************************************************************/
unsigned int
hashtable_count(hashtable_t *h)
{
	if(!h)
		return 0;
    return h->nodecount;
}

/*****************************************************************************/
int
hashtable_insert(hashtable_t *h, void *v)
{
	unsigned int index;
    
	if(!h)
		return 0;
    /* This method allows duplicate keys - but they shouldn't be used */
    if (++(h->nodecount) > h->loadlimit)
    {
        /* Ignore the return value. If expand fails, we should
         * still try cramming just this value into the existing table
         * -- we may not have memory for a larger table, but one more
         * element may be ok. Next time we insert, we'll try expanding again.*/
        //hashtable_expand(h);
    }
  
    index = indexfor(h->tablelength,hash(h,v));

	//memcpy(v+h->offset, &(h->table[index]), sizeof(void *));
    *(unsigned long *)(v+h->offset)  = (unsigned long)h->table[index];
    h->table[index] = v;
    return -1;
}

/*****************************************************************************/
void * /* returns value associated with key */
hashtable_search(hashtable_t *h, void *v)
{
	void *e;
    unsigned int hashvalue, index;

	if(!h)
		return NULL;
    
    hashvalue = hash(h,v);
    index = indexfor(h->tablelength,hashvalue);
    e = h->table[index];
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == hash(h,e)) && (h->compfunc(v, e))) return e;
        e = (void *)*(unsigned long *)(e+h->offset);
    }
    return NULL;
}

/*****************************************************************************/
int /* returns 1 success || 0 error */
hashtable_remove(hashtable_t *h, void *v)
{
	void *e, **pE;
    
	if(!h)
		return 0;
    /* TODO: consider compacting the table when the load factor drops enough,
     *       or provide a 'compact' method. */
    unsigned int hashvalue, index;

    hashvalue = hash(h,v);
    index = indexfor(h->tablelength,hashvalue);
    pE = &(h->table[index]);
    e = *pE;
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == hash(h,e)) && (h->compfunc(v,e)))
        {
            *pE = (void *)*(unsigned long *)(e+h->offset);
            h->nodecount--;
            h->free_value(e);
            return 1;
        }
        pE = e+h->offset;
        e = (void *)*(unsigned long *)(e+h->offset);
    }
    return 0;
}

/*****************************************************************************/
/* destroy */
void
hashtable_free(hashtable_t *h)
{
	unsigned int i;
    void *e, *f, **table;
    table = h->table;
    
	if(!h)
		return;
   
    for (i = 0; i < h->tablelength; i++)
    {
        e = table[i];
        while (NULL != e)
        { 
        	f = e; 
		e = (void *)*(unsigned long *)(e+h->offset);
		h->free_value(f); 
	  }
    }
    
    free(h);
}



/******************************************************************************/
/* hashtable_trace
 * function to output the values in hashtable by turn
 */
void
hashtable_trace(hashtable_t *h)
{
	int i;
   	void *e;
   	
	if(!h) 
		return;
	
	printf("/******************Hashtable_trace********************/\n\n");
	printf("hashtable->nodecount : %u\nhashtable->loadlimit : %u\nhashtable->tablelength : %u\n\n",h->nodecount,h->loadlimit,h->tablelength);
	printf("====================table cotents====================\n");
	
	
    	for(i = 0; i < h->tablelength; i++)
    	{
    		e = h->table[i];
    		printf("table[%d] :\n",i);
		while (NULL != e)
		{
		    printf("\tentry->value :%p\n",e);
		    e = (void *)*(unsigned long *)(e+h->offset);
		}
	}
    return;
}


