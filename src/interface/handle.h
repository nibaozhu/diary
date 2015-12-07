/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef HANDLE_H
#define HANDLE_H

#if 0
#include <openssl/evp.h>
#endif

#include "transport.h"

int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *interface, Transport* t);
int checkid(const void *ptr, size_t size);
int checksum(const void *ptr, size_t size, char *md_value_0, char *digestname);

#endif

