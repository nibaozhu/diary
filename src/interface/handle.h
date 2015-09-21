#ifndef HANDLE_H
#define HANDLE_H

#include "application.h"

int handle(enum Type t, void *message, int size);

int handle_json();
int handle_protobuf();
int handle_xml();
int handle_kv();

#endif

