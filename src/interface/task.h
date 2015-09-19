#ifndef TASK_H
#define TASK_H

#include <queue>
#include <string>
#include <map>

#include "transport.h"
#include "handle.h"

int init(int argc, char **argv);
int task(int argc, char **argv);
int task_r(std::queue<Transport*> *r, std::map<int, Transport*> *m);
int task_x(std::queue<Transport*> *r, std::queue<Transport*> *w, std::map<int, Transport*> *m);
int task_w(std::queue<Transport*> *w);

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

# endif
