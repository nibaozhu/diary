/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Written by Ni Baozhu: <nibz@qq.com>.
 */

#include "logging.h"

const char *level[debug + 1] = {
	"EMERGENCY",
	"ALERT    ",
	"CRITICAL ",
	"ERROR    ",
	"WARNING  ",
	"NOTICE   ",
	"INFO     ",
	"DEBUG    ",
};

const char *color[debug + 1] = {
	"\e[31;1m", // red
	"\e[35;1m", // purple
	"\e[33;1m", // yellow
	"\e[34;1m", // blue
	"\e[36;1m", // cyan
	"\e[32;1m", // green
	"\e[37;1m", // white(gray)
	"\e[37;0m", // white(gray, 'no highlight')
};

const char *clear_color = "\e[0m";
struct logging *l;

int pflush(void)
{
#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s, fflush, l->stream = %p, l->cache= %u, l->cache_max = %u\n",
		level[debug], __FILE__, __LINE__, __func__, "tracing", l->stream, l->cache, l->cache_max);
#endif
	if (l == NULL)
	{
		fprintf(stderr, "%s %s:%d: %s: l = %p\n", level[error], __FILE__, __LINE__, __func__, l);
		return EXIT_FAILURE;
	}

	if (l->stream_level == none)
	{
		return EXIT_SUCCESS;
	}

	// The function fflush() forces a write of all user-space buffered data for the given output or update stream via the stream’s
	//        underlying write function.
	int retval = fflush(l->stream);
	if (retval == EOF)
	{
		fprintf(stderr, "%s %s:%d: %s: %s(%u)\n", level[error], __FILE__, __LINE__, __func__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	// Clean cache when fflush is success.
	l->cache = 0;

	long size = 0;
	// The ftell() function obtains the current value of the file position indicator for the stream pointed to by stream.
	size = ftell(l->stream);
	if (size == -1)
	{
		fprintf(stderr, "%s %s:%d: %s: %s(%u)\n", level[error], __FILE__, __LINE__, __func__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	l->size = size;
#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s, l->size = %lu, l->size_max = %lu\n",
			level[debug], __FILE__, __LINE__, __func__, "tracing", l->size, l->size_max);
#endif
	if (l->size < l->size_max)
	{
		return EXIT_SUCCESS;
	}

	retval = fclose(l->stream);
	if (retval == EOF)
	{
		fprintf(stderr, "%s %s:%d: %s: %s(%u)\n", level[error], __FILE__, __LINE__, __func__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	char oldpath[PATH_MAX];
	char newpath[PATH_MAX];
	memset(oldpath, 0, sizeof oldpath);
	memset(newpath, 0, sizeof newpath);
	snprintf(oldpath, sizeof oldpath, "%s/%s_%u.log.%u.tmp", l->path, l->name, l->pid, l->number);
	snprintf(newpath, sizeof newpath, "%s/%s_%u.log.%u", l->path, l->name, l->pid, l->number);

	retval = rename(oldpath, newpath);
	if (retval == -1)
	{
		fprintf(stderr, "%s %s:%d: %s: %s(%u)\n", level[error], __FILE__, __LINE__, __func__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	char path[PATH_MAX]; // logging file's path
	memset(path, 0, sizeof path);
	snprintf(path, sizeof path, "%s/%s_%u.log.%u.tmp", l->path, l->name, l->pid, ++l->number);

	FILE *fp = fopen(path, l->mode);
	if (fp == NULL)
	{
		fprintf(stderr, "%s %s:%d: %s: path = \"%s\", %s(%u)\n",
			level[error], __FILE__, __LINE__, __func__, path, strerror(errno), errno);
		return EXIT_FAILURE;
	}
	l->stream = fp;

	return EXIT_SUCCESS;
}

int plog(enum elevel x, const char *fmt, ...)
{
#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s, l = %p\n",
		level[debug], __FILE__, __LINE__, __func__, "tracing", l);
#endif
	if (l == NULL)
	{
		fprintf(stderr, "%s %s:%d: %s: l == NULL\n", level[error], __FILE__, __LINE__, __func__);
		return EXIT_FAILURE;
	}

	char str[DATE_MAX];
	sysdate(str);

	if (x <= l->stdout_level)
	{
		fprintf(stdout, "%s %s%-10s%s|", str, color[x], level[x], clear_color);
	}

	if (x <= l->stream_level)
	{
		fprintf(l->stream, "%s %-10s|", str, level[x]);
	}

	va_list ap;
	va_start(ap, fmt);
	if (x <= l->stdout_level)
	{
		vfprintf(stdout, fmt, ap);
	}
	va_end(ap);

	va_start(ap, fmt);
	if (x <= l->stream_level)
	{
		vfprintf(l->stream, fmt, ap);
	}
	va_end(ap);

	if (l->cache_max == 0)
	{
		; //...
	}
	else if (++l->cache < l->cache_max)
	{
#ifdef DEBUG
		fprintf(stdout, "%s %s:%d: %s: %s, l->cache = %u, l->cache_max = %u\n",
			level[debug], __FILE__, __LINE__, __func__, "tracing", l->cache, l->cache_max);
#endif
		return EXIT_SUCCESS;
	}

	struct tm t0;
	struct timeval t1;

	// gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)).
	gettimeofday(&t1, NULL);

	// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
	//	on January 1, 1970, Coordinated Universal Time (UTC).
	localtime_r(&t1.tv_sec, &t0);
	time_t diff = mktime(&t0) - mktime(&l->t1);
#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s, diff = %lu seconds, %04d-%02d-%02d %02d:%02d:%02d ->- %04d-%02d-%02d %02d:%02d:%02d\n",
			level[debug], __FILE__, __LINE__, __func__, "tracing", 
			diff,
			t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec,
			l->t1.tm_year + 1900, l->t1.tm_mon + 1, l->t1.tm_mday, l->t1.tm_hour, l->t1.tm_min, l->t1.tm_sec
		);
#endif
	if (diff < l->diff)
	{
		return EXIT_SUCCESS; // no flush
	}

	int retval = 0;
	retval = pflush();

	assert(retval == EXIT_SUCCESS);
	// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
	//	on January 1, 1970, Coordinated Universal Time (UTC).
	localtime_r(&t1.tv_sec, &l->t1);
	return retval;
}

int initializing(void)
{
#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s, l = %p\n",
		level[debug], __FILE__, __LINE__, __func__, "tracing", l);
#endif
	if (l == NULL)
	{
		fprintf(stderr, "%s %s:%d: %s: l == NULL\n", level[error], __FILE__, __LINE__, __func__);
		return EXIT_FAILURE;
	}

	if (l->stream_level == none)
	{
		return EXIT_SUCCESS;
	}

	if (l->size_max == 0)
	{
		l->size_max = SIZE_MAX;
	}

	int retval = 0;
	// F_OK, R_OK, W_OK, X_OK test whether the file exists and grants read, write, and execute permissions.
	// Warning: R_OK maybe not needed.
	retval = access(l->path, F_OK | W_OK | X_OK);
	if (retval == -1)
	{
		fprintf(stderr, "%s %s:%d: %s: path = \"%s\", %s(%u)\n",
			level[error], __FILE__, __LINE__, __func__, l->path, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	char path[PATH_MAX];
	memset(path, 0, sizeof path);
	l->number = 0;
	snprintf(path, sizeof path, "%s/%s_%u.log.%u.tmp", l->path, l->name, l->pid, ++l->number);

	FILE *fp = fopen(path, l->mode);
	if (fp == NULL)
	{
		fprintf(stderr, "%s %s:%d: %s: path = \"%s\", %s(%u)\n",
			level[error], __FILE__, __LINE__, __func__, path, strerror(errno), errno);
		return EXIT_FAILURE;
	}
	l->stream = fp;

#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s\n", level[debug], __FILE__, __LINE__, __func__, "passed");
#endif

	// print the program name , pid, release
	plog(info, "PROGRAM: %s, PID: %u, RELEASE: %s %s\n", l->name, l->pid, __DATE__, __TIME__);
	return EXIT_SUCCESS;
}

int uninitialized(void)
{
#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s, l = %p\n",
		level[debug], __FILE__, __LINE__, __func__, "tracing", l);
#endif
	if (l == NULL)
	{
		fprintf(stderr, "%s %s:%d: %s: l = %p\n", level[error], __FILE__, __LINE__, __func__, l);
		return EXIT_FAILURE;
	}

	int retval = 0;

	// The function fflush() forces a write of all user-space buffered data for the given output or update stream via the stream’s
	//        underlying write function.
	retval = fflush(l->stream);
	if (retval == EOF)
	{
		fprintf(stderr, "%s %s:%d: %s: %s(%u)\n", level[error], __FILE__, __LINE__, __func__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	// The fclose() function will flushes the stream pointed to by fp (writing any buffered output data using fflush(3)) and closes
	//        the underlying file descriptor.
	retval = fclose(l->stream);
	if (retval == EOF)
	{
		fprintf(stderr, "%s %s:%d: %s: %s(%u)\n", level[error], __FILE__, __LINE__, __func__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	char oldpath[PATH_MAX];
	char newpath[PATH_MAX];
	memset(oldpath, 0, sizeof oldpath);
	memset(newpath, 0, sizeof newpath);
	snprintf(oldpath, sizeof oldpath, "%s/%s_%u.log.%u.tmp", l->path, l->name, l->pid, l->number);
	snprintf(newpath, sizeof newpath, "%s/%s_%u.log.%u", l->path, l->name, l->pid, l->number);

	retval = rename(oldpath, newpath);
	if (retval == -1)
	{
		fprintf(stderr, "%s %s:%d: %s: %s(%u)\n", level[error], __FILE__, __LINE__, __func__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	fprintf(stdout, "%s %s:%d: %s: %s\n", level[debug], __FILE__, __LINE__, __func__, "passed");
#endif
	return EXIT_SUCCESS;
}

int sysdate(char *str)
{
	// format: year-month-day hour:minute:second microsecond
	struct tm t0;
	struct timeval t1;
	size_t size = DATE_MAX;

	// fill memory with a constant byte
	memset(str, 0, size);

	// gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)).
	gettimeofday(&t1, NULL);

	// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
	//	on January 1, 1970, Coordinated Universal Time (UTC).
	localtime_r(&t1.tv_sec, &t0);

	// The function snprintf() writes at most size bytes (including the trailing null byte ('\0')) to str.
	return snprintf(str, size, "%04d-%02d-%02d %02d:%02d:%02d.%06ld", 
				t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec);
}
