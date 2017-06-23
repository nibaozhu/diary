/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Written by Ni Baozhu: <nibz@qq.com>.
 */

#ifndef LOGGING_H
#define LOGGING_H

#ifdef __cplusplus
extern "C" {
/* Limit of `size_t' type.  */
# if __WORDSIZE == 64
#  define SIZE_MAX      (18446744073709551615UL)
# else
#  define SIZE_MAX      (4294967295U)
# endif
#endif

#include <assert.h>
#include <errno.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MODE_MAX (4)
#define DATE_MAX (32)
#define WAITING_SPACE (1) /* seconds */

#define LOGGING_INTERVAL (0)
#define LOGGING_CACHE (BUFSIZ)
#define LOGGING_SIZE ((1<<20)-(1<<10))

enum level {
	none = -1,		/* none logging */

	emergency,		/* application is unusable */
	alert,			/* action must be taken immediately */
	critical,		/* critical conditions */
	error,			/* error conditions */
	warning,		/* warning conditions */
	notice,			/* normal but significant condition */
	info,			/* informational */
	debug,			/* debug-level messages */
};

typedef struct {
	char name[NAME_MAX]; // program name
	struct tm stime; // start time
	struct tm ltime; // the last flush stream time
	time_t diff_max; // time interval
	pid_t pid; // program process id
	unsigned int cache_max; // cache_max lines in memory
	unsigned int cache; // logging has cache lines in memory
	unsigned long size_max; // size_max bytes in file
	unsigned long size; // logging has written bytes in file
	unsigned int number; // logging file's suffix number when splits file

	char path[PATH_MAX]; // logging file's path
	char mode[MODE_MAX]; // logging file's mode
	FILE *stream;

	enum level stdout_level;
	enum level stream_level;
} logging;

/* do not use */
int __logging(enum level x, const char *__file, unsigned int __line, const char *__func, const char *__restrict fmt, ...) __attribute__ ((__format__ (__printf__, 5, 6)));

int initializing(const char *name, const char *path, const char *mode, enum level stream_level, enum level stdout_level, time_t diff_max, unsigned int cache_max, unsigned long size_max);
#define LOGGING(x, fmt, ...) (__logging(x, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__))
int uninitialized();

#ifdef __cplusplus
}
#endif

#endif // logging.h
