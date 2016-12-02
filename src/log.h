#ifndef _LOG_H_
#define _LOG_H_

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

typedef struct log_s log_t;
typedef void (*log_cleanup_cb)(log_t *log);

typedef enum
{
	Log_emerg = 0,
	Log_error = 1,
	Log_alert = 2,
	Log_warning = 3,
	Log_info = 4,
	Log_debug
} level_t;

struct log_s
{
	char *path;
	char *suffix;

	char hour[20];
	FILE *fp;
	level_t level;

	pthread_spinlock_t lock;
	log_cleanup_cb cleanup;
};

// #ifndef LOG_LEVEL
// #define LOG_LEVEL LOG_DEBUG
// #endif

#define log_emerg(log, ...)												\
	do{																	\
		pthread_spin_lock(&log->lock);									\
		log_inner(log, Log_emerg, __FILE__, __LINE__, __VA_ARGS__);		\
		pthread_spin_unlock(&log->lock);								\
	} while(0)

#define log_error(log, ...)												\
	do{																	\
		pthread_spin_lock(&log->lock);									\
		log_inner(log, Log_error, __FILE__, __LINE__, __VA_ARGS__);		\
		pthread_spin_unlock(&log->lock);								\
	} while(0)

#define log_alert(log, ...)												\
	do{																	\
		pthread_spin_lock(&log->lock);									\
		log_inner(log, Log_alert, __FILE__, __LINE__, __VA_ARGS__);		\
		pthread_spin_unlock(&log->lock);								\
	} while(0)

#define log_warn(log, ...)												\
	do{																	\
		pthread_spin_lock(&log->lock);									\
		log_inner(log, Log_warning, __FILE__, __LINE__, __VA_ARGS__);	\
		pthread_spin_unlock(&log->lock);								\
	} while(0)

#define log_info(log, ...)												\
	do{																	\
		pthread_spin_lock(&log->lock);									\
		log_inner(log, Log_info, __FILE__, __LINE__, __VA_ARGS__);		\
		pthread_spin_unlock(&log->lock);								\
	} while(0)

#define log_debug(log, ...)												\
	do{																	\
		pthread_spin_lock(&log->lock);									\
		log_inner(log, Log_debug, __FILE__, __LINE__, __VA_ARGS__);		\
		pthread_spin_unlock(&log->lock);								\
	} while(0)

log_t *log_create(const char *path, const char *suffix, level_t level);

extern int log_inner(log_t *log, level_t level, const char *file,
		const long line, const char *fmt, ...);

void log_destroy(log_t *log);

#endif /*_LOG_H_*/
