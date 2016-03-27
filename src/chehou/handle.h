/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef HANDLE_H
#define HANDLE_H

#include "transport.h"


int handle(Transport* t, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *__m);

void parse_variable_value(std::map<std::string, std::string> *variable_value, const char* src, size_t size);
void business(Transport *t, std::string request_location, std::map<std::string, std::string> *variable_value);

enum {
	INSERT_PRODUCT,
	DELETE_PRODUCT,
	SELECT_PRODUCT,
	UPDATE_PRODUCT,
};

#endif

