/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Written by Ni Baozhu: <nibz@qq.com>.
 */
#include "logging.h"

const char *level[debug + 1][2] = {
{ /* red         */ "\e[31m", "emergency0" },
{ /* purple      */ "\e[35m", "alert1" },
{ /* yellow      */ "\e[33m", "critical2" },
{ /* blue        */ "\e[34m", "error3" },
{ /* cyan        */ "\e[36m", "warning4" },
{ /* green       */ "\e[32m", "notice5" },
{ /* white(gray) */ "\e[37m", "info6" },
{ /* white(gray) */ "\e[37m", "debug7" }}; /* Number stands for level. */
static char *stop = "\e[0m";
static logging l;
pthread_mutex_t __mutex = PTHREAD_MUTEX_INITIALIZER;

static int __timestamp(char *str) {
	struct tm t0;
	struct timeval t1;
	gettimeofday(&t1, NULL);
	localtime_r(&t1.tv_sec, &t0);

#ifdef  __USE_BSD
#ifdef    HH_MI_SS_XXXXXX
	return snprintf(str, DATE_MAX, "%02d:%02d:%02d.%06ld %s", 
			t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec, t0.tm_zone);
#else
	return snprintf(str, DATE_MAX, "%04d-%02d-%02d %02d:%02d:%02d.%06ld %s", 
			t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec, t0.tm_zone);
#endif // HH_MI_SS_XXXXXX
#else
#ifdef    HH_MI_SS_XXXXXX
	return snprintf(str, DATE_MAX, "%02d:%02d:%02d.%06ld", 
			t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec);
#else
	return snprintf(str, DATE_MAX, "%04d-%02d-%02d %02d:%02d:%02d.%06ld", 
			t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec, t1.tv_usec);
#endif // HH_MI_SS_XXXXXX
#endif // __USE_BSD
}

static int __fflush(void) {
#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, fflush, l.stream = %p, l.cache= %u, l.cache_max = %u\n",
			level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing", l.stream, l.cache, l.cache_max);
#endif
	if (l.stream_level == none) return 0;

	int ret = fflush(l.stream);
	if (ret == EOF) LOGGING_TRACING;
	l.cache = 0;

	long size = ftell(l.stream);
	if (size == -1) LOGGING_TRACING;

	l.size = size;
#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, l.size = %lu, l.size_max = %lu\n",
			level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing", l.size, l.size_max);
#endif
	struct tm t0;
	struct timeval t1;
	gettimeofday(&t1, NULL);
	localtime_r(&t1.tv_sec, &t0);

	bool reset_number = false;
	if (t0.tm_year != l.ltime.tm_year || t0.tm_mon  != l.ltime.tm_mon  || t0.tm_mday != l.ltime.tm_mday)
		reset_number = true;
	else if (l.size < l.size_max) return 0;

	ret = fclose(l.stream);
	if (ret == EOF) LOGGING_TRACING;

	char newpath[PATH_MAX] = { 0 }, oldpath[PATH_MAX] = { 0 };
	snprintf(newpath, PATH_MAX, "%s/%s", l.path, l.final_file);
	snprintf(oldpath, PATH_MAX, "%s.%s", newpath, l.file_subfix);

	if (reset_number) l.number = 0;

	ret = access(newpath, F_OK);
	if (ret == -1 && errno != ENOENT) LOGGING_TRACING;
	else if (ret == 0) LOGGING_TRACING;

	ret = rename(oldpath, newpath);
	if (ret == -1) LOGGING_TRACING;

	localtime_r(&t1.tv_sec, &(l.ltime));
	snprintf(l.final_file, NAME_MAX, "%s_%04d-%02d-%02d_%u.%u.log", l.name, 
		l.ltime.tm_year + 1900, l.ltime.tm_mon + 1, l.ltime.tm_mday, l.pid, ++l.number);

	char path[PATH_MAX] = { 0 };
	snprintf(path, PATH_MAX, "%s/%s.%s", l.path, l.final_file, l.file_subfix);

	FILE *fp = fopen(path, l.mode);
	if (fp == NULL) LOGGING_TRACING;
	l.stream = fp;
	return 0;
}

int __logging(enum level x, 
	const char *__file, unsigned int __line, const char *__func, 
	const char *fmt, ...) {
	int ret = pthread_mutex_lock(&__mutex);
	if (ret != 0) LOGGING_TRACING;

#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s\n",
			level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing");
#endif
	struct tm t0;
	struct timeval t1;
	gettimeofday(&t1, NULL);
	localtime_r(&t1.tv_sec, &t0);
	time_t diff = mktime(&t0) - mktime(&l.ltime);

#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s, %04d-%02d-%02d %02d:%02d:%02d => %04d-%02d-%02d %02d:%02d:%02d, diff = %lu seconds\n",
			level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing", 
			l.ltime.tm_year + 1900, l.ltime.tm_mon + 1, l.ltime.tm_mday, l.ltime.tm_hour, l.ltime.tm_min, l.ltime.tm_sec, 
			t0.tm_year + 1900, t0.tm_mon + 1, t0.tm_mday, t0.tm_hour, t0.tm_min, t0.tm_sec, diff);
#endif
	if (t0.tm_year != l.ltime.tm_year || t0.tm_mon != l.ltime.tm_mon || t0.tm_mday != l.ltime.tm_mday) {
		ret = __fflush();
		localtime_r(&t1.tv_sec, &l.ltime);
	}

	char str[DATE_MAX];
	__timestamp(str);

	pid_t tid = syscall(SYS_gettid);

	if (x <= l.stdout_level)
		x <= warning ?
			fprintf(stdout, "%s%s %s %d (%s:%d:%s)%s ", level[x][0], str, level[x][1], tid, __file, __line, __func, stop):
			fprintf(stdout, "%s%s %s %d%s ", level[x][0], str, level[x][1], tid, stop);

	if (x <= l.stream_level)
		x <= warning?
			fprintf(l.stream, "%s %s %d (%s:%d:%s) ", str, level[x][1], tid, __file, __line, __func):
			fprintf(l.stream, "%s %s %d ", str, level[x][1], tid);

	va_list ap;
	va_start(ap, fmt);
	if (x <= l.stdout_level) vfprintf(stdout, fmt, ap);
	va_end(ap);

	va_start(ap, fmt);
	if (x <= l.stream_level) {
		vfprintf(l.stream, fmt, ap);
		if (x <= warning) {
			ret = __fflush();
			localtime_r(&t1.tv_sec, &l.ltime);
		}
	}
	va_end(ap);

	do {
		if (diff + 1 < l.diff_max) break;
		if (l.cache_max == 0) ;
		else if (++l.cache < l.cache_max) {
#ifndef NDEBUG
			fprintf(stdout, "%s%s%s %s:%d: %s: %s, l.cache = %u, l.cache_max = %u\n",
					level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing", l.cache, l.cache_max);
#endif
			break; // no fflush
		}

		ret = __fflush();
		localtime_r(&t1.tv_sec, &l.ltime);
	} while (0);
	ret = pthread_mutex_unlock(&__mutex);
	if (ret != 0) LOGGING_TRACING;
	return 0;
}

int initializing(const char *name, const char *path, const char *mode, 
	enum level stream_level, enum level stdout_level, 
	time_t diff_max, unsigned int cache_max, unsigned long size_max) {
#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s\n",
			level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing");
#endif
	const char *ptr = rindex(name, '/');
	ptr == NULL ? strncpy(l.name, name, strlen(name)) : strncpy(l.name, ptr + 1, strlen(ptr));

	l.pid = getpid();
	l.diff_max = diff_max;
	l.cache_max = cache_max;
	l.size_max = size_max;
	strncpy(l.path, path, PATH_MAX);
	strncpy(l.mode, mode, MODE_MAX);
	l.stream_level = stream_level;
	l.stdout_level = stdout_level;
	strncpy(l.file_subfix, "tmp", NAME_MAX);

	struct timeval t0;
	gettimeofday(&t0, NULL);
	localtime_r(&t0.tv_sec, &l.stime);
	memcpy(&(l.ltime), &(l.stime), sizeof (struct tm));

	l.number = 0;
	if (l.stream_level == none) {
		fprintf(stdout, "%s%s%s %s:%d: %s: %s, l.stream_level = %d\n", 
				level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing", l.stream_level);
		return 0;
	}

	if (l.size_max == 0) l.size_max = SIZE_MAX;

	int ret = access(l.path, F_OK | W_OK | X_OK);
	if (ret == -1) LOGGING_TRACING;

	snprintf(l.final_file, NAME_MAX, "%s_%04d-%02d-%02d_%u.%u.log", l.name, 
		l.stime.tm_year + 1900, l.stime.tm_mon + 1, l.stime.tm_mday, l.pid, ++l.number);

	char __path[PATH_MAX] = { 0 };
	snprintf(__path, PATH_MAX, "%s/%s.%s", l.path, l.final_file, l.file_subfix);

	FILE *fp = fopen(__path, l.mode);
	if (fp == NULL) LOGGING_TRACING;
	l.stream = fp;

#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s\n", level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "passed");
#endif
	LOGGING(info, "PROGRAM: %s, PID: %u, RELEASE: %s %s\n", 
		l.name, l.pid, __DATE__, __TIME__);
	return 0;
}

int uninitialized(void) {
#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s\n",
			level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "tracing");
#endif
	if (l.stream_level == none && l.stream == NULL) return 0;

	int ret = fclose(l.stream);
	if (ret == EOF) LOGGING_TRACING;

	char newpath[PATH_MAX] = { 0 }, oldpath[PATH_MAX] = { 0 };
	snprintf(newpath, PATH_MAX, "%s/%s", l.path, l.final_file);
	snprintf(oldpath, PATH_MAX, "%s.%s", newpath, l.file_subfix);

	ret = access(newpath, F_OK);
	if (ret == -1 && errno != ENOENT) LOGGING_TRACING;
	else if (ret == 0) {
		fprintf(stderr, "%s%s%s %s:%d: %s: newpath = '%s' already exists.\n", 
			level[error][0], level[error][1], stop, __FILE__, __LINE__, __func__, newpath);
		return -1;
	}

	ret = rename(oldpath, newpath);
	if (ret == -1) LOGGING_TRACING;

#ifndef NDEBUG
	fprintf(stdout, "%s%s%s %s:%d: %s: %s\n", level[debug][0], level[debug][1], stop, __FILE__, __LINE__, __func__, "passed");
#endif
	return 0;
}
