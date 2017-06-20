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

#include <linux/limits.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include <pthread.h>
extern pthread_mutex_t mutex;

#define MODE_MAX (4)
#define DATE_MAX (32)

enum elevel {
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

extern const char *level[debug + 1];
extern const char *color[debug + 1];
extern const char *clear_color;

struct logging {
	char name[NAME_MAX]; // program name
	struct tm t0; // start time
	struct tm t1; // the last flush stream date/time
	time_t diff; // time interval
	pid_t pid; // program process id
	unsigned int cache_max; // cache_max lines in memory
	unsigned int cache; // logging has cache lines in memory
	unsigned long size_max; // size_max bytes in file
	unsigned long size; // logging has written bytes in file
	unsigned int number; // logging file's suffix number when splits file

	char path[PATH_MAX]; // logging file's path
	char mode[MODE_MAX]; // logging file's mode
	FILE *stream;

	enum elevel stdout_level;
	enum elevel stream_level;
};

int sysdate(char *str);
int initializing(const char *name, const char *path, const char *mode, enum elevel stream_level, enum elevel stdout_level, time_t diff, unsigned int cache_max, unsigned long size_max);
int __plog(enum elevel x, const char *__file, unsigned int __line, const char *__function, const char *__restrict fmt, ...) __attribute__ ((__format__ (__printf__, 5, 6)));
int pflush();
int uninitialized();

#define plog(x, fmt, ...) (__plog(x, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__))

#ifdef __cplusplus
}
#endif

#endif // logging.h
