/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Written by Ni Baozhu: <nibz@qq.com>.
 */

#include "logging.h"

/* Number stands for level. */
const char *level[debug + 1] = 
{
	"emergency[0]",
	"....alert[1]",
	".critical[2]",
	"....error[3]",
	"..warning[4]",
	"...notice[5]",
	".....info[6]",
	"....debug[7]",
};

const char *color[debug + 1] = 
{
	"\e[31m", // red
	"\e[35m", // purple
	"\e[33m", // yellow
	"\e[34m", // blue
	"\e[36m", // cyan
	"\e[32m", // green
	"\e[37m", // white(gray)
	"\e[37m", // white(gray)
};

static char *clear_color = "\e[0m";
static struct logging *l;
static pthread_mutexattr_t __mutexattr;
static pthread_mutex_t __mutex;

static int __timestamp(char *str)
{
	// format: year-month-day hour:minute:second.microsecond
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

#ifdef  __USE_BSD
#ifdef    HH_MI_SS_XXXXXX
	// The function snprintf() writes at most size bytes (including the trailing null byte ('\0')) to str.
	return snprintf(str, size, "%02d:%02d:%02d.%06ld %s", 
			t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec, t0.tm_zone);
#else
	// The function snprintf() writes at most size bytes (including the trailing null byte ('\0')) to str.
	return snprintf(str, size, "%04d-%02d-%02d %02d:%02d:%02d.%06ld %s", 
			t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec, t0.tm_zone);
#endif // HH_MI_SS_XXXXXX
#else
#ifdef    HH_MI_SS_XXXXXX
	// The function snprintf() writes at most size bytes (including the trailing null byte ('\0')) to str.
	return snprintf(str, size, "%02d:%02d:%02d.%06ld %z", 
			t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec, t0.__tm_zone);
#else
	// The function snprintf() writes at most size bytes (including the trailing null byte ('\0')) to str.
	return snprintf(str, size, "%04d-%02d-%02d %02d:%02d:%02d.%06ld %z", 
			t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec, t0.__tm_zone);
#endif // HH_MI_SS_XXXXXX
#endif // __USE_BSD
}

static int __flush()
{
	int ret;
	assert(l != NULL);

#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, fflush, l->stream = %p, l->cache= %u, l->cache_max = %u\n",
			color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", l->stream, l->cache, l->cache_max);
#endif

	if (l->stream_level == none)
	{
		return 0;
	}

	// The function fflush() forces a write of all user-space buffered data for the given output or update stream via the stream's
	//		underlying write function.
	ret = fflush(l->stream);
	if (ret == EOF)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		/* No space left on device */
		if (errno == ENOSPC) {
			fprintf(stderr, "Waiting %d seconds...\n", WAITING_SPACE);
			sleep(WAITING_SPACE);
			return 0;
		}
		return -1;
	}

	// Clean cache when fflush is success.
	l->cache = 0;

	long size = 0;
	// The ftell() function obtains the current value of the file position indicator for the stream pointed to by stream.
	size = ftell(l->stream);
	if (size == -1)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	l->size = size;
#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, l->size = %lu, l->size_max = %lu\n",
			color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", l->size, l->size_max);
#endif

	struct tm t0;
	struct timeval t1;

	// gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)).
	gettimeofday(&t1, NULL);

	// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
	//	on January 1, 1970, Coordinated Universal Time (UTC).
	localtime_r(&t1.tv_sec, &t0);

	bool reset_number = false;
	if (t0.tm_year != l->t1.tm_year || t0.tm_mon != l->t1.tm_mon || t0.tm_mday != l->t1.tm_mday)
	{
		reset_number = true;
		; //...
	}
	else if (l->size < l->size_max)
	{
		return 0;
	}

	ret = fclose(l->stream);
	if (ret == EOF)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	char oldpath[PATH_MAX];
	char newpath[PATH_MAX];
	memset(oldpath, 0, sizeof oldpath);
	memset(newpath, 0, sizeof newpath);
	snprintf(oldpath, sizeof oldpath, "%s/%s_%04d-%02d-%02d_%u.%u.log.tmp", l->path, l->name, l->t1.tm_year + 1900, l->t1.tm_mon + 1, l->t1.tm_mday, l->pid, l->number);
	snprintf(newpath, sizeof newpath, "%s/%s_%04d-%02d-%02d_%u.%u.log", l->path, l->name, l->t1.tm_year + 1900, l->t1.tm_mon + 1, l->t1.tm_mday, l->pid, l->number);

	if (reset_number)
	{
		l->number = 0;
	}

	/* F_OK tests for the existence of the file. */
	ret = access(newpath, F_OK);
	if (ret == -1 && errno != ENOENT)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}
	else if (ret == 0)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: newpath = '%s' already exists!\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, newpath);
		return -1;
	}

	ret = rename(oldpath, newpath);
	if (ret == -1)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	localtime_r(&t1.tv_sec, &(l->t1));
	char path[PATH_MAX]; // logging file's path
	memset(path, 0, sizeof path);
	snprintf(path, sizeof path, "%s/%s_%04d-%02d-%02d_%u.%u.log.tmp", l->path, l->name, l->t1.tm_year + 1900, l->t1.tm_mon + 1, l->t1.tm_mday, l->pid, ++l->number);

	FILE *fp = fopen(path, l->mode);
	if (fp == NULL)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: path = \"%s\", %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, path, strerror(errno), errno);
		return -1;
	}
	l->stream = fp;
	return 0;
}

int __plog(enum elevel x, const char *__file, unsigned int __line, const char *__function, const char *fmt, ...)
{
	int ret = pthread_mutex_lock(&__mutex);
	if (ret != 0)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	assert(l != NULL);
#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, l = %p\n",
			color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", l);
#endif

	struct tm t0;
	struct timeval t1;

	// gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)).
	gettimeofday(&t1, NULL);

	// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
	//	on January 1, 1970, Coordinated Universal Time (UTC).
	localtime_r(&t1.tv_sec, &t0);
	time_t diff = mktime(&t0) - mktime(&l->t1);

#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, %04d-%02d-%02d %02d:%02d:%02d => %04d-%02d-%02d %02d:%02d:%02d, diff = %lu seconds\n",
			color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", 
			l->t1.tm_year + 1900, l->t1.tm_mon + 1, l->t1.tm_mday, l->t1.tm_hour, l->t1.tm_min, l->t1.tm_sec, 
			t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec, 
			diff
		   );
#endif

	if (t0.tm_year != l->t1.tm_year || t0.tm_mon != l->t1.tm_mon || t0.tm_mday != l->t1.tm_mday)
	{
		ret = __flush();
		assert(ret == 0);
		// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
		//	on January 1, 1970, Coordinated Universal Time (UTC).
		localtime_r(&t1.tv_sec, &l->t1);
		; //...
	}

	char str[DATE_MAX];
	__timestamp(str);

	pthread_t thread = pthread_self();
	pid_t tid = syscall(SYS_gettid);

	if (x <= l->stdout_level)
	{
		if (x <= warning)
		{
			fprintf(stdout, "%s%s %s [0x%lx] [%d] (%s:%d:%s)%s ", color[x], str, level[x], thread, tid, __file, __line, __function, clear_color);
		}
		else
		{
			fprintf(stdout, "%s%s %s [0x%lx] [%d]%s ", color[x], str, level[x], thread, tid, clear_color);
		}
	}

	if (x <= l->stream_level)
	{
		if (x <= warning)
		{
			fprintf(l->stream, "%s %s [0x%lx] [%d] (%s:%d:%s) ", str, level[x], thread, tid, __file, __line, __function);
		}
		else
		{
			fprintf(l->stream, "%s %s [0x%lx] [%d] ", str, level[x], thread, tid);
		}
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
		if (x <= warning)
		{
			ret = __flush();
			assert(ret == 0);
			// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
			//	on January 1, 1970, Coordinated Universal Time (UTC).
			localtime_r(&t1.tv_sec, &l->t1);
		}
	}
	va_end(ap);

	do {
		/* error */
		if (diff + 1 < l->diff_max)
		{
			break; // no flush
		}

		if (l->cache_max == 0)
		{
			; // ...
		}
		else if (++l->cache < l->cache_max)
		{
#ifdef DEBUG_LOGGING
			fprintf(stdout, "%s%s%s %s:%d: %s: %s, l->cache = %u, l->cache_max = %u\n",
					color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", l->cache, l->cache_max);
#endif
			break; // no flush
		}

		ret = __flush();
		assert(ret == 0);

		// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
		//	on January 1, 1970, Coordinated Universal Time (UTC).
		localtime_r(&t1.tv_sec, &l->t1);

	} while (0);

	ret = pthread_mutex_unlock(&__mutex);
	if (ret != 0)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}
	return ret;
}

int initializing(const char *name, const char *path, const char *mode, enum elevel stream_level, enum elevel stdout_level, time_t diff_max, unsigned int cache_max, unsigned long size_max)
{
	if (l == NULL) {
		l = (struct logging*)malloc(sizeof (struct logging));
		memset(l, 0, sizeof *l);
	}
#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, l = %p\n",
			color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", l);
#endif

	const char *ptr = rindex(name, '/');
	if (ptr == NULL) {
		strncpy(l->name, name, strlen(name));
	} else {
		strncpy(l->name, ptr + 1, strlen(ptr));
	}

	l->pid = getpid();
	l->diff_max = diff_max;
	l->cache_max = cache_max;
	l->size_max = size_max;
	strncpy(l->path, path, strlen(path));
	strncpy(l->mode, mode, strlen(mode));
	l->stream_level = stream_level;
	l->stdout_level = stdout_level;

	struct timeval t0;
	// gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)).
	gettimeofday(&t0, NULL);

	// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
	//	on January 1, 1970, Coordinated Universal Time (UTC).
	localtime_r(&t0.tv_sec, &l->t0);
	memcpy(&(l->t1), &(l->t0), sizeof (struct tm));

	l->number = 0;
	if (l->stream_level == none)
	{
		fprintf(stdout, "%s%s%s %s:%d: %s: %s, l->stream_level = %d\n", 
				color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", l->stream_level);
		return 0;
	}

	if (l->size_max == 0)
	{
		l->size_max = SIZE_MAX;
	}

	int ret = 0;
	// F_OK, R_OK, W_OK, X_OK test whether the file exists and grants read, write, and execute permissions.
	// Warning: R_OK maybe not needed.
	ret = access(l->path, F_OK | W_OK | X_OK);
	if (ret == -1)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: path = \"%s\", %s(%u)\n",
				color[error], level[error], clear_color, __FILE__, __LINE__, __func__, l->path, strerror(errno), errno);
		return -1;
	}

	char __path[PATH_MAX];
	memset(__path, 0, sizeof __path);
	snprintf(__path, sizeof __path, "%s/%s_%04d-%02d-%02d_%u.%u.log.tmp", l->path, l->name, l->t0.tm_year + 1900, l->t0.tm_mon + 1, l->t0.tm_mday, l->pid, ++l->number);

	FILE *fp = fopen(__path, l->mode);
	if (fp == NULL)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: path = \"%s\", %s(%u)\n",
				color[error], level[error], clear_color, __FILE__, __LINE__, __func__, __path, strerror(errno), errno);
		return -1;
	}
	l->stream = fp;

#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s\n", color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "passed");
#endif

	ret = pthread_mutexattr_init(&__mutexattr);
	if (ret != 0) {
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return -1;
	}

	ret = pthread_mutex_init(&__mutex, &__mutexattr);
	if (ret != 0) {
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return -1;
	}

	// Print the program name, pid, release
	plog(info, "PROGRAM: %s, PID: %u, RELEASE: %s %s\n", l->name, l->pid, __DATE__, __TIME__);
	return 0;
}

int uninitialized()
{
#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, l = %p\n",
			color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "tracing", l);
#endif
	assert(l != NULL);

	if (l->stream_level == none && l->stream == NULL) {
		return 0;
	}
	assert(l->stream != NULL);
	int ret = 0;

	// The function fflush() forces a write of all user-space buffered data for the given output or update stream via the stream's
	//		underlying write function.
	ret = fflush(l->stream);
	if (ret == EOF)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	// The fclose() function will flushes the stream pointed to by fp (writing any buffered output data using fflush(3)) and closes
	//		the underlying file descriptor.
	ret = fclose(l->stream);
	if (ret == EOF)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	char oldpath[PATH_MAX];
	char newpath[PATH_MAX];
	memset(oldpath, 0, sizeof oldpath);
	memset(newpath, 0, sizeof newpath);
	snprintf(oldpath, sizeof oldpath, "%s/%s_%04d-%02d-%02d_%u.%u.log.tmp", l->path, l->name, l->t1.tm_year + 1900, l->t1.tm_mon + 1, l->t1.tm_mday, l->pid, l->number);
	snprintf(newpath, sizeof newpath, "%s/%s_%04d-%02d-%02d_%u.%u.log", l->path, l->name, l->t1.tm_year + 1900, l->t1.tm_mon + 1, l->t1.tm_mday, l->pid, l->number);

	/* F_OK tests for the existence of the file. */
	ret = access(newpath, F_OK);
	if (ret == -1 && errno != ENOENT)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}
	else if (ret == 0)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: newpath = '%s' already exists!\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, newpath);
		return -1;
	}

	ret = rename(oldpath, newpath);
	if (ret == -1)
	{
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

#ifdef DEBUG_LOGGING
	fprintf(stdout, "%s%s%s %s:%d: %s: %s\n", color[debug], level[debug], clear_color, __FILE__, __LINE__, __func__, "passed");
#endif
	free(l);
	l = NULL;

	ret = pthread_mutex_destroy(&__mutex);
	if (ret != 0) {
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	ret = pthread_mutexattr_destroy(&__mutexattr);
	if (ret != 0) {
		fprintf(stderr, "%s%s%s %s:%d: %s: %s(%u)\n", color[error], level[error], clear_color, __FILE__, __LINE__, __func__, strerror(errno), errno);
		return -1;
	}

	return 0;
}
