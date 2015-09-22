/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef TASK_H
#define TASK_H

#include <queue>
#include <string>
#include <map>

#include "transport.h"
#include "handle.h"

int setnonblocking(int fd);
int reads(Transport *t);
int writes(Transport *t);

void handler(int signum);
void set_disposition(void);

int init(int argc, char **argv);
int uninit(std::map<int, Transport*> *m);

int task(int argc, char **argv);
int task_r(std::queue<Transport*> *r, std::map<int, Transport*> *m);
int task_x(std::queue<Transport*> *r, std::queue<Transport*> *w, std::map<int, Transport*> *m);
int task_w(std::queue<Transport*> *w);

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#endif
