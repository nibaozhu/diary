/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "task.h"
#include "version.h"

#define MAX_EVENTS (1<<8)
#define BUFFER_LENGTH (1<<8)
struct epoll_event ev, events[MAX_EVENTS];
int listen_sock, conn_sock, nfds, epollfd;

extern char *optarg;
extern int optind, opterr, optopt;
extern struct logging *l;
bool is_quit;
short int port = 12340;
char ip[3 + 1 + 3 + 1 + 3 + 1 + 3 + 1] = "0.0.0.0";

int setnonblocking(int fd) {
	int ret = 0;
	plog(debug, "%s:%d:%s\n", __FILE__, __LINE__, __func__);
	do {
		int flags = fcntl(fd, F_GETFL);    
		if (flags == -1) {
			ret = flags;
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}
		flags |= O_NONBLOCK; /* If the O_NONBLOCK flag is enabled, 
								then the system call fails with the error EAGAIN. */
		ret = fcntl(fd, F_SETFL, flags);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}
	} while (0);
	return ret;
}

int reads(Transport *t) {
	int ret = 0;
	int fd = t->get_fd();
	void *buffer = malloc(BUFFER_LENGTH + 1);
	size_t rl = 0;
	time_t t0 = 0, t1 = 0;
	double speed = 0;
	assert(t != NULL);
	t0 = time(NULL);
	do {
		memset(buffer, 0, BUFFER_LENGTH + 1);
		ret = read(fd, buffer, BUFFER_LENGTH);
		if (ret < 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		} else if (ret == 0) {
			t->set_alive(false);
			plog(notice, "Client active closed.(End of File)\n");
			break;
		} else if (ret > 0 && ret <= BUFFER_LENGTH) {
			rl += ret;
			t->set_rx(buffer, ret);
			memset(buffer, 0, ret);

			if (ret > 0 && ret < BUFFER_LENGTH) {
				break;
			}
		}
	} while (1);
	t1 = time(NULL);
	if (t1 - t0 <= 0.000001) {
		speed = -1;
	} else {
		speed = rl * 1. / ((t1 - t0) * 1024 * 1024);
		plog(notice, "speed: %0.2f M/s\n", t->set_speed(speed));
	}
	free(buffer);
	return ret;
}

int writes(Transport *t) {
	int ret = 0;
	assert(t != NULL);
	do {
		int fd = t->get_fd();
		if (t->get_wp() <= 0) {
			break;
		}

		t->pw();
		ret = write(fd, t->get_wx(), t->get_wp());
		if (ret < 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		} else if (ret >= 0 && (size_t)ret <= t->get_wp()) {
			/* Moving forward. */
			memmove(t->get_wx(), (const void *)((char *)t->get_wx() + ret), t->get_wp() - ret);
			t->set_wp(t->get_wp() - ret);
		}
	} while (0);
	return ret;
}

void handler(int signum) {
	plog(notice, "Received signal %d\n", signum);
	switch (signum) {
		case SIGQUIT: /* shortcut: Ctrl + \ */
		case SIGINT : /* shortcut: Ctrl + C */
		case SIGTERM:
			is_quit = true;
			break;
		case SIGUSR1:
		case SIGSEGV:
			is_quit = true;
			break;
		case SIGUSR2:
			pflush();
			break;
		default:
			plog(warning, "Undefined handler.");
	}
	return ;
}

void set_disposition(void) {
	int arr[] = {SIGQUIT, SIGINT, SIGUSR1, SIGUSR2, SIGTERM, SIGSEGV};
	size_t i = 0;
	int signum = 0;
	for ( ; i < sizeof arr / sizeof (int); i++) {
		signum = arr[i];
		if (signal(signum, handler) == SIG_ERR) {
			plog(error, "set the disposition of the signal(signum = %d) to handler.\n", signum);
		}
	}
	return ;
}

int init(int argc, char **argv) {
	srand(getpid());
	int ret = 0;
	do {
		const char *optstring = "hvi:p:";
		int opt;

		while ((opt = getopt(argc, argv, optstring)) != -1) {
			switch (opt) {
				case 'v':
					printf("%s %s %s\n", version, __DATE__, __TIME__);
					exit(0);
				case 'i':
					memcpy(ip, optarg, sizeof ip);
					break;
				case 'p':
					port = atoi(optarg);
					break;
				case 'h':
				default: /* ? */
					printf("Usage: %s [-hv] [-i ip] [-p port]\n", argv[0]);
					printf("ip = %s, port = %d\n", ip, port);
					exit(0);
			}
		}

		l = (struct logging*) malloc(sizeof (struct logging));
		memset(l, 0, sizeof *l);

		char *name;
		name = rindex(argv[0], '/');
		if (name == NULL) {
			strncpy(l->name, argv[0], sizeof l->name - 1);
		} else {
			strncpy(l->name, name + 1, sizeof l->name - 1);
		}

		struct timeval t0;
		// gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)).
		gettimeofday(&t0, NULL);

		// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
		//	on January 1, 1970, Coordinated Universal Time (UTC).
		localtime_r(&t0.tv_sec, &l->t0);

		struct tm t2;
		t2.tm_year = 0;
		t2.tm_mon = 0;
		t2.tm_mday = 0;
		t2.tm_hour = 0;
		t2.tm_min = 0;
		t2.tm_sec = 1;

		l->diff = t2.tm_sec + t2.tm_min * 60 + t2.tm_hour * 60 * 60 + t2.tm_mday * 60 * 60 * 24 + t2.tm_mon * 60 * 60 * 24 * 30 + t2.tm_year * 60 * 60 * 24 * 30 * 365;
		l->pid = getpid();
		l->cache_max = 1;
		// l->size_max = 1024*1024*1; // 1MB
		l->size_max = 0;
		strncpy(l->path, "../../log", sizeof l->path - 1);
		strncpy(l->mode, "w+", sizeof l->mode - 1);
		l->stream_level = debug;
		l->stdout_level = debug;

		ret = initializing();
		if (ret == EXIT_FAILURE) {
			ret = -1;
			break;
		}

		is_quit = false;
		set_disposition();
		listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock < 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof (struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		ret = inet_pton(AF_INET, ip, (struct sockaddr *) &addr.sin_addr.s_addr);
		if (ret != 1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		socklen_t addrlen = sizeof (struct sockaddr_in);
		ret = bind(listen_sock, (struct sockaddr *) &addr, addrlen);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		int backlog = (1<<4);
		ret = listen(listen_sock, backlog);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		epollfd = epoll_create(MAX_EVENTS);
		if (epollfd == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP; /* epoll edge triggered */
		ev.data.fd = listen_sock; /* bind & listen's fd */
		ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		plog(info, "Assigning address {%s:%u}.\n", ip, port);
		plog(info, "Refer to by sockfd = %d as a passive socket.\n", listen_sock);
		plog(info, "Epoll file descriptor = %d.\n", epollfd);
	} while (0);
	return ret;
}

int uninit(std::map<int, Transport*> *m) {
	int ret = 0;
	do {
		if (listen_sock > 0) {
			plog(info, "Close listen socket.\n");
			ret = close(listen_sock);
			if (ret == -1) {
				plog(error, "%s(%d)\n", strerror(errno), errno);
			}
		}

		if (epollfd > 0) {
			plog(info, "Close epoll socket.\n");
			ret = close(epollfd);
			if (ret == -1) {
				plog(error, "%s(%d)\n", strerror(errno), errno);
			}
		}

		std::map<int, Transport*>::iterator i = m->begin();
		while (i != m->end()) {
			delete (*i).second;
			m->erase(i++);
		}

		ret = uninitialized();
		free(l);

	} while (0);
	return ret;
}

int task(int argc, char **argv) {
	int ret = 0;
	std::list<Transport*> *r = new std::list<Transport*>();
	std::list<Transport*> *w = new std::list<Transport*>();
	std::map<int, Transport*> *m = new std::map<int, Transport*>();

	do {
		ret = init(argc, argv);
		if (ret == -1) {
			break;
		}

		while (true) {
			ret = task_r(r, m);
			ret = task_x(r, w, m);
			ret = task_w(w);
			if (is_quit) {
				break;
			}
		}
	} while (0);
	ret = uninit(m);
	return ret;
}

int task_r(std::list<Transport*> *r, std::map<int, Transport*> *m) {
	int ret = 0;
	assert(r != NULL);
	assert(m != NULL);

	do {
		struct sockaddr_in peer_addr;
		socklen_t peer_addrlen = sizeof (struct sockaddr_in);
		memset(&peer_addr, 0, sizeof (struct sockaddr_in));

		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
		if (nfds == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		for (int n = 0; n < nfds; n++) {
			if (events[n].data.fd == listen_sock) {
				int acceptfd = accept(listen_sock, (struct sockaddr *) &peer_addr, &peer_addrlen);
				if (acceptfd == -1) {
					plog(error, "%s(%d)\n", strerror(errno), errno);
					break;
				}

				time_t created = time(NULL);
				plog(notice, "acceptfd = %d\n", acceptfd);
				/* set non blocking */
				ret = setnonblocking(acceptfd);
				if (ret == -1) {
					break;
				}

				ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP; /* edge triggered */
				ev.data.fd = acceptfd;
				ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptfd, &ev);
				if (ret == -1) {
					plog(error, "%s(%d)\n", strerror(errno), errno);
					break;
				}

				plog(notice, "Someone connected to me.\n");
				Transport *t = new Transport(acceptfd, created, peer_addr, peer_addrlen);
				(*m)[acceptfd] = t;

			} else {
				plog(debug, "events[%d].events = 0x%04x\n", n, events[n].events);
				if (events[n].events & EPOLLERR) {
					plog(error, "Error condition happened on the associated file descriptor.\n");
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						plog(debug, "%s(%d)\n", strerror(errno), errno);
						continue;
					}
					ret = close(events[n].data.fd);
					if (ret == -1) {
						plog(error, "%s(%d)\n", strerror(errno), errno);
						continue;
					}

					delete (*m)[events[n].data.fd];
					m->erase(events[n].data.fd);
					continue;
				}

				if (events[n].events & EPOLLHUP) {
					plog(notice, "Hang up happened on the associated file descriptor.\n");
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						plog(error, "%s(%d)\n", strerror(errno), errno);
						continue;
					}
					ret = close(events[n].data.fd);
					if (ret == -1) {
						plog(error, "%s(%d)\n", strerror(errno), errno);
						continue;
					}

					delete (*m)[events[n].data.fd];
					m->erase(events[n].data.fd);
					continue;
				}

				if (events[n].events & EPOLLRDHUP) {
					plog(notice, "Client active closed.\n");
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						plog(error, "%s(%d)\n", strerror(errno), errno);
						continue;
					}
					ret = close(events[n].data.fd);
					if (ret == -1) {
						plog(error, "%s(%d)\n", strerror(errno), errno);
						continue;
					}

					delete (*m)[events[n].data.fd];
					m->erase(events[n].data.fd);
					continue;
				}

				if (events[n].events & EPOLLIN) {
					plog(notice, "The associated file is available for read(2) operations.\n");
					if (m == NULL) {
						plog(error, "m = %p\n", m);
						break;
					}
					Transport *t = (*m)[events[n].data.fd];
					ret = reads(t);
					if (ret < 0) {
						break;
					} else if (ret == 0) {
						;
					}

					/* Push to list. */
					r->push_back(t);
				}

/*
				if (events[n].events & EPOLLOUT) {
					plog(debug, "The associated file is available for write(2) operations.\n");
					if (m == NULL) {
						plog(debug, "m = %p\n", m);
						return ret;
					}
					Transport *t = (*m)[events[n].data.fd];
					ret = writes(t);
					if (ret < 0) {
						break;
					}
				}
*/
			}
		}

	} while (0);
	return ret;
}

int task_w(std::list<Transport*> *w) {
	int ret = 0;
	Transport *t = NULL;
	assert(w != NULL);

	/**
	 *  Returns true if the %list is empty.
	 */
	std::list<Transport*>::iterator i = w->begin();
	while (i != w->end()) {
		t = *i;
		if (t == NULL) {
			i = w->erase(i);
			continue;
		}

		plog(debug, "wx = %p, wp = %lu\n", t->get_wx(), t->get_wp());
		t->pw();

		ret = writes(t);
		if (t->get_wp() == 0) {
			i = w->erase(i);
		} else {
			i++;
		}
		plog(debug, "Wrote %d bytes\n", ret);
	}
	return ret;
}

int task_x(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m) {
	int ret = 0;
	Transport *t = NULL;
	assert(r != NULL);
	assert(w != NULL);
	assert(m != NULL);

	/**
	 *  Returns true if the %list is empty.
	 */
	std::list<Transport*>::iterator i = r->begin();
	while (i != r->end()) {
		t = *i;
		ret = handle(t, m, w);
		if (ret == -1) {
			plog(error, "\n");
			// continue;
		}
		i = r->erase(i);
	}
	return ret;
}
