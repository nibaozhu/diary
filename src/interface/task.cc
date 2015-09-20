#include "task.h"

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
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}
		flags |= O_NONBLOCK; //non block
		ret = fcntl(fd, F_SETFL, flags);
		if (ret == -1) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}
	} while (0);
	return ret;
}

// int reads(int fd) {
int reads(Transport *t) {
	int ret = 0;
	int fd = t->get_fd();
	if (t == NULL) {
		printf("t = %p\n", t);
		return ret;
	}

	printf("fd = %d, %s\n", fd, __func__);
	do {
		char buffer[BUFFER_LENGTH];
		memset(buffer, 0, sizeof buffer);
		ret = read(fd, buffer, BUFFER_LENGTH);
		if (ret < 0) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}
		else if (ret == 0) {
			;;;;
			break;
		}
		printf("[%s]\n", buffer);

		if (ret > 0) {
			t->set_data(buffer, ret + 1);
			memset(buffer, 0, ret + 1);
		}

	} while (1);
	return ret;
}

// int writes(int fd) {
int writes(Transport *t) {
	int ret = 0;
	int fd = t->get_fd();
	if (t == NULL) {
		printf("t = %p\n", t);
		return ret;
	}

	printf("fd = %d, %s\n", fd, __func__);
	do {
		char buffer[BUFFER_LENGTH];
		memset(buffer, 0, sizeof buffer);
		sprintf(buffer, "send");
		size_t length = strlen(buffer);
		ret = write(fd, buffer, length);
		if (ret < 0) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}
		printf("[%s]\n", buffer);
	} while (0);
	return ret;
}

int init(int argc, char **argv) {
	int ret = 0;
	do {
		listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock < 0) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}

		printf("listen_sock = %d\n", listen_sock);

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof (struct sockaddr_in));

		addr.sin_family = AF_INET;

#ifdef D
		addr.sin_port = htons(atoi("12340"));
		ret = inet_pton(AF_INET, "127.0.0.1", (struct sockaddr *) &addr.sin_addr.s_addr);
#else
		addr.sin_port = htons(atoi(argv[2]));
		ret = inet_pton(AF_INET, argv[1], (struct sockaddr *) &addr.sin_addr.s_addr);
#endif

		if (ret != 1) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}

		socklen_t addrlen = sizeof (struct sockaddr_in);
		ret = bind(listen_sock, (struct sockaddr *) &addr, addrlen);
		if (ret == -1) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}

		int backlog = 5; /* may be wrong */
		ret = listen(listen_sock, backlog);
		if (ret == -1) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}

		epollfd = epoll_create(MAX_EVENTS);
		if (epollfd == -1) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}
		printf("epollfd = %d\n", epollfd);

		ev.events = EPOLLIN | EPOLLET; /* epoll edge triggered */
		ev.data.fd = listen_sock; /* bind & listen's fd */
		ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev);
		if (ret == -1) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}
	} while (0);
	return ret;
}

int task(int argc, char **argv) {
	int ret = 0;
	std::queue<Transport*> *r = new std::queue<Transport*>();
	std::queue<Transport*> *w = new std::queue<Transport*>();
	std::map<int, Transport*> *m = new std::map<int, Transport*>();

	ret = init(argc, argv);
	if (ret == -1) {
		return ret;
	}

	while (true) {
		ret = task_r(r, m);
		ret = task_x(r, w, m);
		ret = task_w(w);
	}
	return ret;
}

int task_r(std::queue<Transport*> *r, std::map<int, Transport*> *m) {
	int ret = 0;
	do {
		struct sockaddr_in peer_addr;
		socklen_t peer_addrlen = sizeof (struct sockaddr_in);
		memset(&peer_addr, 0, sizeof (struct sockaddr_in));

		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
		if (nfds == -1) {
			printf("%s(%d)\n", strerror(errno), errno);
			break;
		}

		for (int n = 0; n < nfds; n++) {
			if (events[n].data.fd == listen_sock) {
				int acceptfd = accept(listen_sock, (struct sockaddr *) &peer_addr, &peer_addrlen);
				if (acceptfd == -1) {
					printf("%s(%d)\n", strerror(errno), errno);
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
					printf("%s(%d)\n", strerror(errno), errno);
					break;
				}

				;;;;;;;;;;;;;;;;;;;;
				printf("some one connect to me\n");
				;;;;;;;;;;;;;;;;;;;;

				Transport *t = new Transport(acceptfd);
				(*m)[acceptfd] = t;

			} else {
				if (events[n].events & EPOLLHUP) {
					printf("events[%d].events = 0x%03x\n", n, events[n].events);
					puts("Hang up happened on the associated file descriptor."); // my message
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						printf("%s(%d)\n", strerror(errno), errno);
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
						printf("%s(%d)\n", strerror(errno), errno);
						break;
					}
					close(events[n].data.fd);
					continue;
				}

				if (events[n].events & EPOLLIN) {
					printf("events[%d].events = 0x%03x\n", n, events[n].events);
					puts("The associated file is available for read(2) operations."); // my message

					// ret = reads(events[n].data.fd);
					// Transport *t = new Transport();
					// t->set_fd(events[n].data.fd);

					if (m == NULL) {
						printf("m = %p\n", m);
						return ret;
					}
					Transport *t = (*m)[events[n].data.fd];
					ret = reads(t);
					if (ret < 0) {
						break;
					}

					/* Now, we need push to queue. */
					r->push(t);
				}

				if (events[n].events & EPOLLOUT) {
					printf("events[%d].events = 0x%03x\n", n, events[n].events);
					puts("The associated file is available for write(2) operations."); // my message

					// ret = writes(events[n].data.fd);
					// Transport *t = new Transport();
					// t->set_fd(events[n].data.fd);

					if (m == NULL) {
						printf("m = %p\n", m);
						return ret;
					}
					Transport *t = (*m)[events[n].data.fd];
					ret = writes(t);
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
	if (w == NULL) {
		printf("w = %p\n", w);
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

int task_x(std::queue<Transport*> *r, std::queue<Transport*> *w, std::map<int, Transport*> *m) {
	int ret = 0;
	if (r == NULL) {
		printf("r = %p, w = %p, m = %p\n", r, w, m);
		return ret;
	}

	;;;;;;;;;;;;;;;;;;;;
	/**
	 *  Returns true if the %queue is empty.
	 */
	while (!r->empty()) {
		printf("i should handle data, size = %d\n", r->size());

		Transport *t = r->front();
		printf("data = %s\n", t->get_data());

		r->pop();
		printf("i am handling data\n");
	}
	;;;;;;;;;;;;;;;;;;;;

	return ret;
}
