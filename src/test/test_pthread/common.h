#ifndef COMMON_H
#define COMMON_H

#include "logging.h"

typedef struct {
	size_t department_ID;
	size_t employee_ID;

#ifdef UUID_LEN_STR
	const char UUID[UUID_LEN_STR + 1]; /* Universal Unique ID */
#endif

} personal_information_t;

#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */



#endif // COMMON_H
