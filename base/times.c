#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

int time2str(char *tmbuf, size_t size, const char *fmt)
{
	time_t local;
	struct tm now;

	local = time(NULL);
	localtime_r(&local, &now);
	strftime(tmbuf, size, fmt, &now);

	return 0;
}
