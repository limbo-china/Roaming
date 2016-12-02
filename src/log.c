#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "log.h"
#include "times.h"

log_t *log_create(const char *path, const char *suffix, level_t level)
{
	log_t *log;
	char fname[128] =
	{ 0 };

	log = (log_t *) calloc(sizeof(log_t), 1);
	if (log == NULL)
	{
		perror("log_create");
		return NULL;
	}

	time2str(log->hour, sizeof(log->hour), "%Y-%m-%d-%H");

	log->path = strdup(path);
	if (log->path == NULL)
	{
		perror("strdup");
		return NULL;
	}

	log->suffix = strdup(suffix);
	if (log->suffix == NULL)
	{
		perror("strdup");
		return NULL;
	}

	sprintf(fname, "%s/%s.%s", path, log->hour, suffix);
	log->fp = fopen(fname, "a");
	if (log->fp == NULL)
	{
		perror("fopen");
		return NULL;
	}
	log->level = level;

	pthread_spin_init(&log->lock, PTHREAD_PROCESS_PRIVATE);

	log->cleanup = log_destroy;
	return log;
}

inline int log_inner(log_t *log, level_t level, const char *file,
		const long line, const char *fmt, ...)
{
	va_list args;
	FILE *fp = NULL;

	char tmstr[20], fname[128];

	if (level > log->level)
		return 1;

	time2str(tmstr, 20, "%Y-%m-%d-%H:%M:%S");

	switch (level)
	{
	case Log_emerg:
		fprintf(log->fp, "[%s]-[EMERG]-", tmstr);
		printf("[%s]-[EMERG]-", tmstr);
		break;
	case Log_error:
		fprintf(log->fp, "[%s]-[ERROR]-", tmstr);
		printf("[%s]-[ERROR]-", tmstr);
		break;
	case Log_alert:
		fprintf(log->fp, "[%s]-[ALERT]-", tmstr);
		printf("[%s]-[ALERT]-", tmstr);
		break;
	case Log_warning:
		fprintf(log->fp, "[%s]-[WARN]-", tmstr);
		printf("[%s]-[WARN]-", tmstr);
		break;
	case Log_info:
		fprintf(log->fp, "[%s]-[INFO]-", tmstr);
		printf("[%s]-[INFO]-", tmstr);
		break;
	case Log_debug:
		fprintf(log->fp, "[%s]-[DEBUG]-%s:%ld-", tmstr, file, line);
		printf("[%s]-[DEBUG]-%s:%ld-", tmstr, file, line);
		break;
	default:
		break;
	}

	va_start(args, fmt);
	vfprintf(log->fp, fmt, args);
	va_end(args);

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	
	fflush(log->fp);

	if (strncmp(log->hour, tmstr, 13))
	{
		memset(log->hour, '\0', 20);
		strncpy(log->hour, tmstr, 13);

		sprintf(fname, "%s/%s.%s", log->path, log->hour, log->suffix);
		if ((fp = fopen(fname, "a")) != NULL)
		{
			fclose(log->fp);
			log->fp = fp;
		}
		else
		{
			perror("fopen");
		}
	}

	return 0;
}

void log_destroy(log_t *log)
{
	if (log->fp)
		fclose(log->fp);

	if (log->path)
		free(log->path);

	if (log->suffix)
		free(log->suffix);

	if (log)
		free(log);
}
