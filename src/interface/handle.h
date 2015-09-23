/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef HANDLE_H
#define HANDLE_H

#include "transport.h"

int handle(Transport *t, std::queue<Transport*> *w);
int md5sum(const char *ptr, int size);

#define LENGTH (8)
#define MD5SUM_LENGTH (32)
#define IDENTIFICATION_LENGTH (8)

#endif

