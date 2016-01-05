/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef HANDLE_H
#define HANDLE_H

#include "transport.h"


int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *is, Transport* t);

#endif

