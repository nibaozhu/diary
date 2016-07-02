#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>

#include <list>
#include <map>

#include <unistd.h>
#include <errno.h>

#include <string.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/epoll.h>

#include <fcntl.h>
#include <cstdio>
#include <cstdlib>

#define MAXEVENTS (1<<4)

enum {
	ERROR_socket_bind_listen_epoll,
};

#endif /* COMMON_H */
