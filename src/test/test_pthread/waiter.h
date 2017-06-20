#ifndef WAITER_H
#define WAITER_H


#include <stdio.h>
#include <pthread.h>

#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */



void *waiter(void *arg);


#include "logging.h"

#endif
