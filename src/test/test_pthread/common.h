#ifndef COMMON_H
#define COMMON_H

#include "logging.h"

#include <sys/queue.h>
#include <uuid.h>

typedef struct task_s {

	size_t ID;

#ifdef UUID_LEN_STR
	char UUID[UUID_LEN_STR + 1]; /* Universal Unique ID */
#endif

	char path[PATH_MAX];

	SLIST_ENTRY(task_s) entry;
} task_t;

typedef struct {
	size_t department_ID;
	size_t employee_ID;

#ifdef UUID_LEN_STR
	const char UUID[UUID_LEN_STR + 1]; /* Universal Unique ID */
#endif

	SLIST_HEAD(task_slist_s, task_s) *task_slist;
} personal_information_t;

#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */



#endif // COMMON_H
