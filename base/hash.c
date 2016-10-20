#ifndef _HASH_H_
#define _HASH_H_

#include <string.h>
#include <stdint.h>

unsigned int time33_hash(unsigned char *data, size_t len)
{
	unsigned int hash = 0;

	while(*data)
		hash = (hash << 5) + hash + (*data++);

	return hash;
}

unsigned int murmur_hash(unsigned char *data, size_t len)
{
	unsigned int hash, temp;

	hash = 0 ^ len;

	while(len >= 4) 
	{
		temp  = data[0];
		temp |= data[1] << 8;
		temp |= data[2] << 16; 
		temp |= data[3] << 24; 

		temp *= 0x5bd1e995;
		temp ^= temp >> 24; 
		temp *= 0x5bd1e995;

		hash *= 0x5bd1e995;
		hash ^= temp;

		data += 4;
		len -= 4;
	}   

	switch(len) 
	{
		case 3:
			hash ^= data[2] << 16; 
		case 2:
			hash ^= data[1] << 8;
		case 1:
			hash ^= data[0];
			hash *= 0x5bd1e995;
	}   

	hash ^= hash >> 13; 
	hash *= 0x5bd1e995;
	hash ^= hash >> 15; 

	return hash;
}

#endif /*_HASH_H_*/
