#ifndef COMMON_H
#define COMMON_H

#include "logging.h"

#include <errno.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <uuid.h>

#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <sys/queue.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <unistd.h>

#include <signal.h>


enum HOTEL_STAFF
{
	CHAIRMAN,
	GENERAL_MANAGER,
	CEO,
	CLEANERS,
	COOK,
	DISHWASHER,
	LOBBY_MANAGER,
	RECEPTION,
	TYPIST,
	WAITER,
	// ...
};

typedef struct hotel_s {

	char hotel_name[NAME_MAX];

	/* Human Resources */
	size_t staff_number; /* default 10 */

	/* Management */
	size_t chairman_number; /* only one */
	size_t general_manager_number; /* only one */
	size_t CEO_number; /* Chief Execute Officer: only one */

	/* Junior */
	size_t cleaners_number; /* only one */
	size_t cook_number; /* only one */
	size_t dishwasher_number; /* only one */
	size_t lobby_manager_number; /* only one */
	size_t reception_number; /* only one */
	size_t typist_number; /* only one */
	size_t waiter_number; /* default x */


	/* Logging */
	size_t diff_max; /* time interval */
	size_t cache_max; /* line cache */
	size_t size_max; /* file size */

	/* */
	pthread_t *pthread;

} hotel_t;

typedef struct task_s {

	size_t ID;

#ifdef UUID_LEN_STR
	char UUID[UUID_LEN_STR + 1]; /* Universal Unique ID */
#endif

	char path[PATH_MAX];

	pid_t ppid; /* the process ID of the parent of the calling process */
	pid_t pid; /* the process ID of the calling process */
	pid_t tid; /* the  caller’s thread ID (TID) */
	pthread_t ptid; /* the ID of the calling thread(This is the same value that is returned in *thread in
       the pthread_create(3) call that created this thread.) */

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


#endif // COMMON_H
