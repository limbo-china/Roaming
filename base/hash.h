#ifndef _HASH_H_
#define _HASH_H_

#include <string.h>

/**
 * @brief	time33 hash alogrithm
 * @param	data	string 
 * @param	len		string len
 * @return	string hash value
 */
unsigned int time33_hash(unsigned char *data, size_t len);

/**
 * @brief	murmur hash alogrithm
 * @param	data	string 
 * @param	len		string len
 * @return	string hash value
 */
unsigned int murmur_hash(unsigned char *data, size_t len);

#endif /*_HASH_H_*/
