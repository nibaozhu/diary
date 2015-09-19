#include "task.h"
#include "handle.h"

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

#define MAX_EVENTS (0xff)
#define BUFFER_LENGTH (0xff)
struct epoll_event ev, events[MAX_EVENTS];
int listen_sock, conn_sock, nfds, epollfd;

int setnonblocking(int fd) {
	int ret = 0;
	printf("%s:%d:%s\n", __FILE__, __LINE__, __func__);
	do {
		int flags = fcntl(fd, F_GETFL);    
		if (flags == -1) {
			ret = flags;
			printf("%s\n", strerror(errno));
			break;
		}
		flags |= O_NONBLOCK; //non block
		ret = fcntl(fd, F_SETFL, flags);
		if (ret == -1) {
			printf("%s\n", strerror(errno));
			break;
		}
	} while (0);
	return ret;
}

int reads(int fd) {
	printf("%s:%d:%s\n", __FILE__, __LINE__, __func__);
	int ret = 0;
	do {
		char buffer[BUFFER_LENGTH];
		memset(buffer, 0, sizeof buffer);
		ret = read(fd, buffer, BUFFER_LENGTH);
		if (ret < 0) {
			printf("%s\n", strerror(errno));
			break;
		}
		printf("fd = %d, READ [%s]\n", fd, buffer);
	} while (0);
	return ret;
}

int writes(int fd) {
	printf("%s:%d:%s\n", __FILE__, __LINE__, __func__);
	int ret = 0;
	do {
		char buffer[BUFFER_LENGTH];
		memset(buffer, 0, sizeof buffer);
		sprintf(buffer, "send");
		size_t length = strlen(buffer);
		ret = write(fd, buffer, length);
		if (ret < 0) {
			printf("%s\n", strerror(errno));
			break;
		}
		printf("fd = %d, write [%s]\n", fd, buffer);
	} while (0);
	return ret;
}

int init(int argc, char **argv) {
	int ret = 0;
	do {
		int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock < 0) {
			printf("%s\n", strerror(errno));
			break;
		}

		printf("listen_sock = %d\n", listen_sock);

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof (struct sockaddr_in));

		addr.sin_family = AF_INET;

#ifdef D
		addr.sin_port = htons(atoi(argv[2]));
		ret = inet_pton(AF_INET, argv[1], (struct sockaddr *) &addr.sin_addr.s_addr);
#else
		addr.sin_port = htons(atoi("12340"));
		ret = inet_pton(AF_INET, "127.0.0.1", (struct sockaddr *) &addr.sin_addr.s_addr);
#endif

		if (ret != 1) {
			printf("%s\n", strerror(errno));
			break;
		}

		socklen_t addrlen = sizeof (struct sockaddr_in);
		ret = bind(listen_sock, (struct sockaddr *) &addr, addrlen);
		if (ret == -1) {
			printf("%s\n", strerror(errno));
			break;
		}

		int backlog = 5; /* may be wrong */
		ret = listen(listen_sock, backlog);
		if (ret == -1) {
			printf("%s\n", strerror(errno));
			break;
		}

		epollfd = epoll_create(MAX_EVENTS);
		if (epollfd == -1) {
			printf("%s\n", strerror(errno));
			break;
		}
		printf("epollfd = %d\n", epollfd);

		ev.events = EPOLLIN | EPOLLET; /* epoll edge triggered */
		ev.data.fd = listen_sock; /* bind & listen's fd */
		ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev);
		if (ret == -1) {
			printf("%s\n", strerror(errno));
			break;
		}
	} while (0);
	return ret;
}

int task(int argc, char **argv) {
	int ret = 0;
	std::queue<Transport*> *r = NULL;
	std::queue<Transport*> *w = NULL;
	std::map<std::string, Transport*> *m = NULL;

	ret = init(argc, argv);
	if (ret == -1) {
		return ret;
	}

	while (true) {
		ret = task_r(r);
		ret = task_x(r, w, m);
		ret = task_w(w);
	}
	return ret;
}

int task_r(std::queue<Transport*> *r) {
	int ret = 0;
	do {
		struct sockaddr_in peer_addr;
		socklen_t peer_addrlen = sizeof (struct sockaddr_in);
		memset(&peer_addr, 0, sizeof (struct sockaddr_in));

		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
		if (nfds == -1) {
			printf("%s\n", strerror(errno));
			break;
		}

		for (int n = 0; n < nfds; n++) {
			if (events[n].data.fd == listen_sock) {
				int acceptfd = accept(listen_sock, (struct sockaddr *) &peer_addr, &peer_addrlen);
				if (acceptfd == -1) {
					printf("%s\n", strerror(errno));
					break;
				}

				printf("accept: acceptfd = %d\n", acceptfd);
				//set non blocking
				ret = setnonblocking(acceptfd);
				if (ret == -1) {
					break;
				}

				ev.events = EPOLLIN | EPOLLET; //epoll edge triggered
				//                  ev.events = EPOLLIN | EPOLLOUT | EPOLLET; //epoll edge triggered
				//                  ev.events = EPOLLIN | EPOLLOUT; //epoll level triggered (default)
				ev.data.fd = acceptfd;
				ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptfd, &ev);
				if (ret == -1) {
					printf("%s\n", strerror(errno));
					break;
				}

				;;;;;;;;;;;;;;;;;;;;
				printf("some one connect to me\n");
				;;;;;;;;;;;;;;;;;;;;

			} else {
				if (events[n].events & EPOLLHUP) {
					printf("events[%d].events = 0x%03x\n", n, events[n].events);
					puts("Hang up happened on the associated file descriptor."); // my message
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1)
					{
						printf("%s\n", strerror(errno));
						break;
					}
					close(events[n].data.fd);

					;;;;;;;;;;;

					continue;
				}

				if (events[n].events & EPOLLERR) {
					printf("0x%03x\n", events[n].events);
					puts("Error condition happened on the associated file descriptor.  "
						"epoll_wait(2) will always wait for this event; it is not necessary to set it in events."); // my message
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						printf("%s\n", strerror(errno));
						break;
					}
					close(events[n].data.fd);
					continue;
				}

				if (events[n].events & EPOLLIN) {
					printf("events[%d].events = 0x%03x\n", n, events[n].events);
					puts("The associated file is available for read(2) operations."); // my message
					ret = reads(events[n].data.fd);
					if (ret < 0) {
						break;
					}
				}

				if (events[n].events & EPOLLOUT) {
					printf("events[%d].events = 0x%03x\n", n, events[n].events);
					puts("The associated file is available for write(2) operations."); // my message
					ret = writes(events[n].data.fd);
					if (ret < 0) {
						break;
					}
				}
			}

			;;;;;;;;;;;;;;;;;;;;
			printf("some data send to me\n");
			;;;;;;;;;;;;;;;;;;;;

		}

	} while (0);
	return ret;
}

int task_w(std::queue<Transport*> *w) {
	int ret = 0;

	printf("w = %p\n", w);
	if (w == NULL) {
		return ret;
	}

	;;;;;;;;;;;;;;;;;;;;
	/**
	 *  Returns true if the %queue is empty.
	 */
	while (!w->empty()) {
		printf("i should send data to some one\n");
	}
	;;;;;;;;;;;;;;;;;;;;

	return ret;
}

int task_x(std::queue<Transport*> *r, std::queue<Transport*> *w, std::map<std::string, Transport*> *m) {
	int ret = 0;

	printf("r = %p, w = %p, m = %p\n", r, w, m);
	if (r == NULL) {
		return ret;
	}

	;;;;;;;;;;;;;;;;;;;;
	/**
	 *  Returns true if the %queue is empty.
	 */
	while (!r->empty()) {
		printf("i should handle data\n");
		printf("i should handle data, size = %d\n", r->size());
	}
	;;;;;;;;;;;;;;;;;;;;

	return ret;
}
