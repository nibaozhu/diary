#ifndef WORKER_H
#define WORKER_H


#include <stdio.h>
#include <pthread.h>

#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */



void *worker(void *arg);


#endif
