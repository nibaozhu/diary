/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef TASK_H
#define TASK_H

#include "transport.h"
#include "handle.h"

int setnonblocking(int fd);
int reads(Transport* t);
void writes(Transport* t);

void handler(int signum);
void set_disposition();

int init(int argc, char **argv);
int uninit(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *__m);

int task(int argc, char **argv);
void task_r(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m);
void task_x(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *__m);
void task_w(std::list<Transport*> *w);

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#endif
