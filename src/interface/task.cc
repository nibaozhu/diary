/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "task.h"
#include "version.h"

#define MAX_EVENTS (1<<8)
#define BUFFER_MAX (1<<8)
struct epoll_event ev, events[MAX_EVENTS];
int listen_sock, conn_sock, nfds, epollfd;
struct sockaddr_in addr;

extern char *optarg;
extern int optind, opterr, optopt;
extern struct logging *l;
bool quit;
bool is_reconfigure;
short int port = 12340;
char ip[3 + 1 + 3 + 1 + 3 + 1 + 3 + 1] = "0.0.0.0";

int setnonblocking(int fd) {
	int ret = 0;
	do {
		int flags = fcntl(fd, F_GETFL);    
		if (flags == -1) {
			ret = -1;
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}
		flags |= O_NONBLOCK; /* If the O_NONBLOCK flag is enabled, then the system call fails with the error EAGAIN. */
		plog(info, "Set the file = %d status flags to the value = 0%o.\n", fd, flags);
		ret = fcntl(fd, F_SETFL, flags);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}
	} while (false);
	return ret;
}

int reads(Transport* t) {
	assert(t != NULL);
	ssize_t ret = 0;
	void *buffer = malloc(BUFFER_MAX + 1);
	size_t rl = 0;
	time_t t0 = 0, t1 = 0;
	double speed = 0;
	t0 = time(NULL);
	do {
		memset(buffer, 0, BUFFER_MAX + 1);
		ret = read(t->get_fd(), buffer, BUFFER_MAX);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		} else if (ret == 0) {
			t->set_alive(false);
			plog(notice, "Stream socket peer = %d closed connection, or shut down writing half of connection.\n", t->get_fd());
			break;
		} else if (ret > 0 && ret <= BUFFER_MAX) {
			rl += ret;
			t->set_rx(buffer, ret);
			memset(buffer, 0, ret);
			if (ret != BUFFER_MAX) {
				break;
			}
		}
	} while (true);
	t1 = time(NULL);
	if (t1 <= t0) {
		speed = -1;
	} else {
		speed = rl * 1. / ((t1 - t0) * 1024 * 1024);
		plog(notice, "speed: %0.2f M/s\n", t->set_speed(speed));
	}
	free(buffer);
	int _ret = ret < 0? -1: 0;
	return _ret;
}

void writes(Transport* t) {
	ssize_t ret = 0;
	assert(t != NULL);
	do {
		t->pw();
		ret = write(t->get_fd(), t->get_wx(), t->get_wp());
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		} else if (ret >= 0 && (size_t)ret <= t->get_wp()) {
			plog(notice, "Wrote 0x%lx bytes to file = %d.\n", ret, t->get_fd());

			/* Moving forward. */
			memmove(t->get_wx(), (const void *)((char *)t->get_wx() + ret), t->get_wp() - ret);
			t->set_wp(t->get_wp() - ret);
		}
	} while (false);
	return ;
}

void handler(int signum) {
	plog(notice, "Received signal %d\n", signum);
	switch (signum) {
		case SIGHUP: /* causes auditd to reconfigure. */
			is_reconfigure = true;
			break;
		case SIGQUIT: /* shortcut: Ctrl + \ */
		case SIGINT : /* shortcut: Ctrl + C */
		case SIGTERM:
			quit = true;
			break;
		case SIGUSR1:
		case SIGSEGV:
			quit = true;
			break;
		case SIGUSR2:
			pflush();
			break;
		default:
			plog(warning, "Undefined handler.");
	}
	return ;
}

void set_disposition() {
	int arr[] = {SIGHUP, SIGQUIT, SIGINT, SIGUSR1, SIGUSR2, SIGTERM/* , SIGSEGV */};
	for (size_t i = 0 ; i < sizeof arr / sizeof (int); i++) {
		int signum = arr[i];
		if (signal(signum, handler) == SIG_ERR) {
			plog(error, "set the disposition of the signal(signum = %d) to handler.\n", signum);
		}
	}
	return ;
}

int init(int argc, char **argv) {
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
					printf( "Usage: %s [OPTION]...\n"
						"Interface server.\n"
						"\n"
						"	-h	Display this help and exit\n"
						"	-v	Output version information and exit\n"
						"	-i IP	Set bind ip, default use %s\n"
						"	-p PORT	Set bind port, default use %d\n"
						"\n"
						"Report %s bugs to %s\n"
						"Home page: <%s>\n"
						"For complete Documentation, see README\n", 
						argv[0], ip, port, argv[0], email, home);
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

		l->diff = 0; // fflush file per 0 seconds
		l->pid = getpid();
		l->cache_max = 1;
		l->size_max = 1024 * 1024 * 10; // 10 MB
		strncpy(l->path, "../../log", sizeof l->path - 1);
		strncpy(l->mode, "w+", sizeof l->mode - 1);
		l->stream_level = debug;
		l->stdout_level = debug;

		ret = initializing();
		if (ret == -1) {
			break;
		}

		plog(info, "%s\n", version);

		quit = false;
		is_reconfigure = false;
		set_disposition();

		epollfd = epoll_create(MAX_EVENTS);
		if (epollfd == -1) {
			plog(emergency, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock < 0) {
			plog(emergency, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		memset(&addr, 0, sizeof (struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		ret = inet_pton(AF_INET, ip, (struct sockaddr *) &addr.sin_addr.s_addr);
		if (ret != 1) {
			plog(emergency, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		socklen_t addrlen = sizeof (struct sockaddr_in);
		ret = bind(listen_sock, (struct sockaddr *) &addr, addrlen);
		if (ret == -1) {
			plog(emergency, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		int backlog = (1<<10);
		ret = listen(listen_sock, backlog);
		if (ret == -1) {
			plog(emergency, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		ev.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
		ev.data.fd = listen_sock; /* bind & listen's fd */
		ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev);
		if (ret == -1) {
			plog(emergency, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		plog(info, "Epoll instance file = %d.\n", epollfd);
		plog(info, "Assigning address %s:%u\n", ip, port);
		plog(info, "Refer to by sockfd = %d as a passive socket.\n", listen_sock);
	} while (false);
	return ret;
}

int uninit(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *inferface) {
	int ret = 0;
	do {
		r->clear();
		delete r;

		w->clear();
		delete w;

		std::map<int, Transport*>::iterator i = m->begin();
		while (i != m->end()) {
			delete i->second;
			m->erase(i++);
		}
		delete m;

		inferface->clear();
		delete inferface;

		if (listen_sock > 0) {
			plog(info, "Close passive file = %d.\n", listen_sock);
			ret = close(listen_sock);
			if (ret == -1) {
				plog(error, "%s(%d)\n", strerror(errno), errno);
			}
		}

		if (epollfd > 0) {
			plog(info, "Close epoll file = %d.\n", epollfd);
			ret = close(epollfd);
			if (ret == -1) {
				plog(error, "%s(%d)\n", strerror(errno), errno);
			}
		}

		ret = uninitialized();
		free(l);
	} while (false);
	return ret;
}

int task(int argc, char **argv) {
	int ret = 0;
	std::list<Transport*> *r = new std::list<Transport*>();
	std::list<Transport*> *w = new std::list<Transport*>();
	std::map<int, Transport*> *m = new std::map<int, Transport*>();
	std::map<uint32_t, int> *interface = new std::map<uint32_t, int>(); // maybe should use multimap
	do {
		srand(getpid());
		ret = init(argc, argv);
		if (ret == -1) {
			break;
		}

		do {
			task_r(r, w, m, interface);

#if 0
			std::map<int, Transport*>::iterator i = m->begin();
			while (i != m->end()) {
				plog(error, "(%d, %p)\n", i->first, i->second);
				i++;
			}
#endif

			task_x(r, w, m, interface);

#if 0
			// std::map<int, Transport*>::iterator i = m->begin();
			i = m->begin();
			while (i != m->end()) {
				plog(error, "(%d, %p)\n", i->first, i->second);
				i++;
			}
#endif

			task_w(w);

#if 0
			// std::map<int, Transport*>::iterator i = m->begin();
			i = m->begin();
			while (i != m->end()) {
				plog(error, "(%d, %p)\n", i->first, i->second);
				i++;
			}
#endif

		} while (!quit);
	} while (false);
	ret = uninit(r, w, m, interface);
	return ret;
}

void task_r(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *interface) {
	assert(r != NULL && m != NULL);
	int ret = 0;
	do {
		struct sockaddr_in peer_addr;
		socklen_t peer_addrlen = sizeof (struct sockaddr_in);
		memset(&peer_addr, 0, sizeof (struct sockaddr_in));

		/* The call waits for a maximum time of timeout milliseconds.  Specifying a timeout of -1 makes epoll_wait() wait indefinitely, while specifying a
		 * timeout equal to zero makes epoll_wait() to return immediately even if no events are available (return code equal to zero). */
		int timeout = 1000;
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
		if (nfds == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		Transport* t = NULL;
		for (int n = 0; n < nfds; n++) {
			if (events[n].data.fd == listen_sock) {
				int acceptfd = accept(listen_sock, (struct sockaddr *) &peer_addr, &peer_addrlen);
				if (acceptfd == -1) {
					plog(error, "%s(%d)\n", strerror(errno), errno);
					break;
				}

				time_t created = time(NULL);
				plog(notice, "It creates a new connected socket = %d.\n", acceptfd);
				ret = setnonblocking(acceptfd); /* Set Non Blocking */
				if (ret == -1) {
					break;
				}

				ev.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
				ev.data.fd = acceptfd;
				ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptfd, &ev);
				if (ret == -1) {
					plog(error, "%s(%d)\n", strerror(errno), errno);
					break;
				}

				char peer_ip[3 + 1 + 3 + 1 + 3 + 1 + 3 + 1];
				memset(peer_ip, 0, sizeof ip);
				strcpy(peer_ip, inet_ntoa(peer_addr.sin_addr));
				plog(notice, "NAME %s:%u->%s:%u\n", ip, htons(addr.sin_port), peer_ip, htons(peer_addr.sin_port));

				t = new Transport(acceptfd, created, peer_addr, peer_addrlen);
				m->insert(std::make_pair(acceptfd, t));
			} else {
				std::map<int, Transport*>::iterator im = m->begin();
				plog(debug, "events[%d].events = 0x%03x\n", n, events[n].events);
				if (events[n].events & EPOLLERR) {
					plog(error, "Error condition happened on the associated file descriptor = %d.\n", events[n].data.fd);
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

#if 0
					plog(emergency, "before: fd = %d, interface->size = %d\n", events[n].data.fd, interface->size());
					std::map<std::string, int>::iterator i = interface->begin();
					while (i != interface->end()) {
						plog(error, "(%s, %d)\n", i->first.c_str(), i->second);
						i++;
					}
#endif

					im = m->find(events[n].data.fd);
					if (im != m->end()) {
						t = im->second;
						w->remove(t);
						interface->erase(t->get_id());
						delete t;
						m->erase(events[n].data.fd);
					}

#if 0
					plog(emergency, "after: fd = %d, interface->size = %d\n", events[n].data.fd, interface->size());
					i = interface->begin();
					while (i != interface->end()) {
						plog(error, "(%s, %d)\n", i->first.c_str(), i->second);
						i++;
					}
#endif

					continue;
				}

				if (events[n].events & EPOLLHUP) {
					plog(notice, "Hang up happened on the associated file descriptor = %d.\n", events[n].data.fd);
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

					im = m->find(events[n].data.fd);
					if (im != m->end()) {
						t = im->second;
						w->remove(t);
						interface->erase(t->get_id());
						delete t;
						m->erase(events[n].data.fd);
					}

					continue;
				}

				if (events[n].events & EPOLLRDHUP) {
					plog(notice, "Stream socket peer = %d closed connection, or shut down writing half of connection.\n", events[n].data.fd);
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

#if 0
					plog(emergency, "before: fd = %d, m->size = %d\n", events[n].data.fd, m->size());
					std::map<int, Transport*>::iterator i = m->begin();
					while (i != m->end()) {
						plog(error, "(%d, %p)\n", i->first, i->second);
						i++;
					}
#endif

					im = m->find(events[n].data.fd);
					if (im != m->end()) {
						t = im->second;
						w->remove(t);
						interface->erase(t->get_id());
						delete t;
						m->erase(events[n].data.fd);
					}

#if 0
					plog(emergency, "end: fd = %d, m->size = %d\n", events[n].data.fd, m->size());
					i = m->begin();
					while (i != m->end()) {
						plog(error, "(%d, %p)\n", i->first, i->second);
						i++;
					}
#endif

					continue;
				}

				if (events[n].events & EPOLLIN) {
					plog(notice, "The associated file = %d is available for read(2) operations.\n", events[n].data.fd);
					if (m == NULL) {
						plog(error, "m = %p\n", m);
						break;
					}
					Transport* t = (*m)[events[n].data.fd];
					ret = reads(t);
					if (ret < 0) {
						break;
					} else if (ret == 0) {
						;
					}
					r->push_back(t);
				}

#if 0
				if (events[n].events & EPOLLOUT) {
					plog(debug, "The associated file is available for write(2) operations.\n");
					if (m == NULL) {
						plog(info, "m = %p\n", m);
						break;
					}
					Transport* t = (*m)[events[n].data.fd];
					writes(t);
				}
#endif
			}
		}

	} while (false);
	return ;
}

void task_w(std::list<Transport*> *w) {
	assert(w != NULL);

	w->sort();
	w->unique();

	std::list<Transport*>::iterator i = w->begin();
	while (i != w->end()) {
		Transport* t = *i;
		if (t == NULL) {
			i = w->erase(i);
			continue;
		}

		writes(t);
		if (t->get_wp() == 0) {
			i = w->erase(i);
		} else {
			i++;
		}
	}
	return ;
}

void task_x(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *interface) {
	assert(r != NULL && w != NULL && m != NULL);
	std::list<Transport*>::iterator i = r->begin();

	r->sort();
	r->unique();

	while (i != r->end()) {
		Transport* t = *i;
		int ret = handle(w, m, interface, t);
		if (ret == -1) {
			plog(error, "Handle fail.\n");
		}
		i = r->erase(i);
	}
	return ;
}

