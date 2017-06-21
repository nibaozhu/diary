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


#endif // COMMON_H
