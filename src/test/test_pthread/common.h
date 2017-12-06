#ifndef COMMON_H
#define COMMON_H

#include <syslog.h>
#include <errno.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ossp/uuid.h>

#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <sys/queue.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <unistd.h>

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <libgen.h>


typedef struct dog_s {
	char name[NAME_MAX];
	size_t number;
	pthread_t *pthread;

	/* worker should gone */
	bool bankruptcy;
} dog_t;
extern dog_t dog;

typedef struct task_s {
	size_t ID;
	char UUID[UUID_LEN_STR+1]; /* Universal Unique ID */

	pid_t ppid; /* the process ID of the parent of the calling process */
	pid_t pid; /* the process ID of the calling process */
	pid_t tid; /* the  caller’s thread ID (TID) */
	pthread_t ptid; /* the ID of the calling thread
		(This is the same value that is returned in *thread in
                 the pthread_create(3) call that created this thread.) */

	SLIST_ENTRY(task_s) entry;
} task_t;

typedef struct {
	size_t ID;
	const char UUID[UUID_LEN_STR+1]; /* Universal Unique ID */

	SLIST_HEAD(task_slist_s, task_s) *task_slist;
} info_t;

#endif // COMMON_H
