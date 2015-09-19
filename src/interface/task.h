#ifndef TASK_H
#define TASK_H

#include <queue>
#include <string>
#include <map>

#include <unistd.h>

#include "transport.h"
int init(int argc, char **argv);
int task(int argc, char **argv);
int task_r(std::queue<Transport*> *r);
int task_x(std::queue<Transport*> *r, std::queue<Transport*> *w, std::map<std::string, Transport*> *m);
int task_w(std::queue<Transport*> *w);

#endif
