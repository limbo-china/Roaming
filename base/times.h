#ifndef _TIMES_H_
#define _TIMES_H_

#include <stdlib.h>

/**
 * @brief convert local time to string
 * @param	tmbuf	time buffer
 * @param	size	time buffer size 
 * @param	fmt		output time by format
 * @return 
 *			0	success
 */
int time2str(char *tmbuf, size_t size, const char *fmt);

#endif /*_TIMES_H_*/
