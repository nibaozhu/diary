/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "task.h"
#include "version.h"

struct epoll_event ev, events[MAX_EVENTS];
int listen_sock, nfds, epollfd;
struct sockaddr_in addr;

extern char *optarg;
extern int optind, opterr, optopt;
bool quit;
bool is_reconfigure;
short int port = 8000;
char ip[3 + 1 + 3 + 1 + 3 + 1 + 3 + 1] = "0.0.0.0";

int setnonblocking(int fd) {
	int ret = 0;
	do {
		int flags = fcntl(fd, F_GETFL);    
		if (flags == -1) {
			ret = -1;
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}
		flags |= O_NONBLOCK; /* If the O_NONBLOCK flag is enabled, then the system call fails with the error EAGAIN. */
		LOGGING(info, "Set the file = %d status flags to the value = 0%o.\n", fd, flags);
		ret = fcntl(fd, F_SETFL, flags);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
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
	struct timeval tv0, tv1;
	double speed = 0;

	/* Begin Time */
	t0 = time(NULL);
	gettimeofday(&tv0, NULL);

	do {
		memset(buffer, 0, BUFFER_MAX + 1);
		ret = read(t->get_fd(), buffer, BUFFER_MAX);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			break;
		} else if (ret == 0) {
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

	/* End Time */
	t1 = time(NULL);
	gettimeofday(&tv1, NULL);

	if (t1 <= t0 && tv1.tv_usec <= tv0.tv_usec) {
		speed = -1;
	} else {									/* 1KiB = 1000B */
		speed = rl * 1000000. / (((t1 - t0) * 1000000. + (tv1.tv_usec - tv0.tv_usec)) * 1000 * 1000);
		t->set_speed(speed);

		LOGGING(notice, "Read 0x%lx bytes from file = %d.\n", rl, t->get_fd());
		if (speed < 1.) {
			//LOGGING(notice, "Download Speed %0.1f KiB/s\n", speed * 1000.);
		} else {
			LOGGING(notice, "Download Speed %0.1f MiB/s\n", speed);
		}
	}
	free(buffer);

	if (ret > INT32_MAX) {
		LOGGING(warning, "ret(0x%lx) > INT32_MAX(0x%x)\n", ret, INT32_MAX);
		ret = INT32_MAX;
	}
	int _ret = labs(ret);
	return _ret;
}

void writes(Transport* t) {
	assert(t != NULL);

	ssize_t ret = 0;
	size_t wl = 0;
	do {
		t->pw();
		ret = write(t->get_fd(), t->get_wx(), t->get_wp());
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			break;
		} else if (ret >= 0 && (size_t)ret <= t->get_wp()) {
			wl = ret;
			LOGGING(notice, "Wrote 0x%lx bytes to file = %d.\n", wl, t->get_fd());

			if (wl != t->get_wp()) {
				ev.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT; /* Level Triggered */
				ev.data.fd = t->get_fd();
				int _ret = epoll_ctl(epollfd, EPOLL_CTL_MOD, ev.data.fd, &ev); /* When wp is zero, we should remove EPOLLOUT event! */
				if (_ret == -1) {
					LOGGING(warning, "%s(%d)\n", strerror(errno), errno);
				}
				t->set_events(ev.events);
			}

			/* Moving forward. */
			memmove(t->get_wx(), (const void *)((char *)t->get_wx() + wl), t->get_wp() - wl);
			t->set_wp(t->get_wp() - wl);
		}
	} while (false);
	return ;
}

void handler(int signum) {
	// FIXME: dead-lock
	// LOGGING(notice, "Received signal %d\n", signum);
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
		case SIGPIPE:
			quit = true;
			break;
		case SIGUSR2:
			// TODO: fflush logging...
			break;
		default:
			;;;
	}
	return ;
}

void set_disposition() {
	int arr[] = {SIGHUP, SIGQUIT, SIGINT, SIGUSR1, SIGUSR2, SIGTERM/* , SIGSEGV */, SIGPIPE};
	for (size_t i = 0 ; i < sizeof arr / sizeof (int); i++) {
		int signum = arr[i];
		if (signal(signum, handler) == SIG_ERR) {
			LOGGING(error, "set the disposition of the signal(signum = %d) to handler.\n", signum);
		}
	}
	return ;
}

int init(int argc, char **argv) {
	int ret = 0;
	do {
		const char *optstring = "ehvi:p:";
		int opt;

		while ((opt = getopt(argc, argv, optstring)) != -1) {
			switch (opt) {
				case 'v':
					printf("%s %s %s\n", version, __DATE__, __TIME__);
					exit(0);
				case 'i':
					strncpy(ip, optarg, sizeof ip - 1);
					break;
				case 'p':
					port = atoi(optarg);
					break;
				case 'e':
					if (fork() > 0) _exit(0);
					setsid(); // creates a session and sets the process group ID
					if (fork() > 0) _exit(0);
					break;
				case 'h':
				default: /* ? */
					printf( "Usage: %s [OPTION]...\n"
							"\n"
							"	-h	Display this help and exit\n"
							"	-v	Output version information and exit\n"
							"	-i IP	Set bind ip, default use %s\n"
							"	-p PORT	Set bind port, default use %d\n"
							"	-e	Daemon\n"
							"\n"
							"Report %s bugs to %s\n"
							"Home page: <%s>\n"
							"For complete Documentation, see README\n", 
							argv[0], ip, port, argv[0], email, home);
					exit(0);
			}
		}

		ret = initializing(argv[0], "logdir", "w+", debug, debug, 1, 1, 1024*1024);
		if (ret == -1) {
			break;
		}

		LOGGING(info, "%s\n", version);

		quit = false;
		is_reconfigure = false;
		set_disposition();

		epollfd = epoll_create(MAX_EVENTS);
		if (epollfd == -1) {
			LOGGING(emergency, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock < 0) {
			LOGGING(emergency, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		memset(&addr, 0, sizeof (struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		ret = inet_pton(AF_INET, ip, (struct sockaddr *) &addr.sin_addr.s_addr);
		if (ret != 1) {
			LOGGING(emergency, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		socklen_t addrlen = sizeof (struct sockaddr_in);
		ret = bind(listen_sock, (struct sockaddr *) &addr, addrlen);
		if (ret == -1) {
			LOGGING(emergency, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		int backlog = (1<<10);
		ret = listen(listen_sock, backlog);
		if (ret == -1) {
			LOGGING(emergency, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		ev.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
		ev.data.fd = listen_sock; /* bind & listen's fd */
		ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
		if (ret == -1) {
			LOGGING(emergency, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		LOGGING(info, "Epoll instance file = %d.\n", epollfd);
		LOGGING(info, "Assigning address %s:%u\n", ip, port);
		LOGGING(info, "Refer to by sockfd = %d as a passive socket.\n", listen_sock);
	} while (false);
	return ret;
}

int uninit(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m, std::list<Transport*> *c) {
	int ret = 0;
	do {
		r->clear();
		delete r;

		w->clear();
		delete w;

		std::map<int, Transport*>::iterator im = m->begin();
		while (im != m->end()) {
			Transport* t = im->second;
			if (!t->get_alive()) {
				int ret = 0;
				LOGGING(info, "Close()  closes a file descriptor = %d(%p), so that it no longer refers to any file and may be reused.\n", im->first, im->second);
				ret = close(t->get_fd());
				if (ret == -1) {
					LOGGING(warning, "%s(%d)\n", strerror(errno), errno);
				}
			}

			delete t;
			m->erase(im++);
		}
		m->clear();
		delete m;

		c->clear();
		delete c;

		if (listen_sock > 0) {
			LOGGING(info, "Close passive file = %d.\n", listen_sock);
			ret = close(listen_sock);
			if (ret == -1) {
				LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			}
		}

		if (epollfd > 0) {
			LOGGING(info, "Close epoll file = %d.\n", epollfd);
			ret = close(epollfd);
			if (ret == -1) {
				LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			}
		}

		ret = uninitialized();
	} while (false);
	return ret;
}

int task(int argc, char **argv) {
	int ret = 0;
	std::list<Transport*> *r = new std::list<Transport*>();
	std::list<Transport*> *w = new std::list<Transport*>();
	std::list<Transport*> *c = new std::list<Transport*>();
	std::map<int, Transport*> *m = new std::map<int, Transport*>(); /* ONE-FILE-DESCRIPTER versus ONE-TRANSPORT-OBJECT */
	do {
		srand(getpid());
		ret = init(argc, argv);
		if (ret == -1) {
			break;
		}

		do {
			task_r(r, w, m);
			task_x(r, w, m, c);
			task_w(w);
		} while (!quit);
	} while (false);
	ret = uninit(r, w, m, c);
	return ret;
}

void task_r(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m) {
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
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		for (int n = 0; n < nfds; n++) {
			Transport* t = NULL;

			if (events[n].data.fd == listen_sock) {
				int acceptfd = accept(listen_sock, (struct sockaddr *) &peer_addr, &peer_addrlen);
				if (acceptfd == -1) {
					LOGGING(error, "%s(%d)\n", strerror(errno), errno);
					break;
				}

				time_t created = time(NULL);
				LOGGING(notice, "It creates a new connected socket = %d.\n", acceptfd);
				ret = setnonblocking(acceptfd); /* Set Non Blocking */
				if (ret == -1) {
					break;
				}

				ev.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
				ev.data.fd = acceptfd;
				ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
				if (ret == -1) {
					LOGGING(error, "%s(%d)\n", strerror(errno), errno);
					break;
				}

				char peer_ip[3 + 1 + 3 + 1 + 3 + 1 + 3 + 1] = {0};
				strncpy(peer_ip, inet_ntoa(peer_addr.sin_addr), sizeof peer_ip - 1);
				LOGGING(notice, "NAME %s:%u->%s:%u\n", ip, htons(addr.sin_port), peer_ip, htons(peer_addr.sin_port));

				t = new Transport(acceptfd, created, peer_addr, peer_addrlen);
				m->insert(std::make_pair(acceptfd, t));
			} else {
				std::map<int, Transport*>::iterator im = m->begin();
				LOGGING(debug, "events[%d].events = 0x%03x\n", n, events[n].events);

				if (events[n].events & EPOLLIN) { /* 0x001 */
					LOGGING(notice, "The associated file = %d is available for read(2) operations.\n", events[n].data.fd);
					Transport* t = (*m)[events[n].data.fd];
					ret = reads(t);
					if (ret < 0) {
						break;
					} else if (ret == 0) {
						;
					} else if (ret > 0) {
						r->push_back(t);
					}
				}

				if (events[n].events & EPOLLOUT) { /* 0x004 */
					LOGGING(notice, "The associated file = %d is available for write(2) operations.\n", events[n].data.fd);
					im = m->find(events[n].data.fd);
					if (im != m->end()) {
						t = im->second;
						w->push_back(t);
					}
				}

				if (events[n].events & EPOLLERR) { /* 0x008 */
					LOGGING(error, "Error condition happened on the associated file descriptor = %d.\n", events[n].data.fd);
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						LOGGING(debug, "%s(%d)\n", strerror(errno), errno);
						continue;
					}

					im = m->find(events[n].data.fd);
					if (im != m->end()) {
						t = im->second;
						t->set_alive(false);
						t->reset();
					}
				}

				if (!(events[n].events & EPOLLERR) && (events[n].events & EPOLLHUP)) { /* 0x010, Except 0x008 */
					LOGGING(notice, "Hang up happened on the associated file descriptor = %d.\n", events[n].data.fd);
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						LOGGING(error, "%s(%d)\n", strerror(errno), errno);
						continue;
					}

					im = m->find(events[n].data.fd);
					if (im != m->end()) {
						t = im->second;
						t->set_alive(false);
					}
				}

				if (!(events[n].events & EPOLLERR) && (events[n].events & EPOLLRDHUP)) { /* 0x2000, Except 0x008 */
					LOGGING(notice, "Stream socket peer = %d closed connection, or shut down writing half of connection.\n", events[n].data.fd);
					ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (ret == -1) {
						LOGGING(error, "%s(%d)\n", strerror(errno), errno);
						continue;
					}

					im = m->find(events[n].data.fd);
					if (im != m->end()) {
						t = im->second;
						t->set_alive(false);
					}
				}

			}
		}
	} while (false);
	return ;
}

void task_x(std::list<Transport*> *r, std::list<Transport*> *w, std::map<int, Transport*> *m, std::list<Transport*> *c) {
	assert(r != NULL && w != NULL && m != NULL);

	time_t ti = time(NULL); /* time() returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds. */
	std::map<int, Transport*>::iterator im = m->begin();
	while (im != m->end()) {
		Transport* t = im->second;
		time_t expired = 120; /* measured in seconds */
		if (!t->get_alive() && ((t->get_rp() == 0 && t->get_wp() == 0) || ti - t->get_updated() >= expired)) {
			int ret = 0;
			LOGGING(info, "Close()  closes a file descriptor = %d(%p), so that it no longer refers to any file and may be reused.\n", im->first, im->second);
			ret = close(t->get_fd());
			if (ret == -1) {
				LOGGING(warning, "%s(%d)\n", strerror(errno), errno);
			}

			r->remove(t);
			w->remove(t);
			m->erase(im++);
			delete t;
		} else {
			im++;
		}
	}

	r->sort();
	r->unique();

	int ret = 0;
	Transport *t = NULL;
	std::list<Transport*>::iterator i;

	i = r->begin();
	while (i != r->end()) {
		t = *i;
		ret = handle(w, m, c, t);
		if (ret == -1) {
			LOGGING(error, "handle fail.\n");
		}
		i = r->erase(i);
	}

	i = c->begin();
	while (i != c->end()) {
		t = *i;
		LOGGING(error, "t = %p\n", t);
		ret = task_c(m, w, t);
		if (ret == -1) {
			LOGGING(error, "task_c fail.\n");
		}
		i = c->erase(i);
	}

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

		if (t->get_wp() == 0 && (t->get_events() & EPOLLOUT)) {
			ev.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
			ev.data.fd = t->get_fd();

			int _ret = epoll_ctl(epollfd, EPOLL_CTL_MOD, ev.data.fd, &ev);
			if (_ret == -1) {
				LOGGING(warning, "%s(%d)\n", strerror(errno), errno);
			}
			t->set_events(ev.events);
		}
		i = w->erase(i);
	}
	return ;
}

int task_c(std::map<int, Transport*> *m, std::list<Transport*> *w, Transport *__t) {
	int ret = 0;

	const char *host = (__t->get_host()).c_str();
	uint16_t port = __t->get_port();

	int domain = AF_INET;
	int type = SOCK_STREAM;
	int protocol = 0;
	int connect_sock = 0;

	do {
		connect_sock = socket(domain, type, protocol);
		if (connect_sock < 0) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		char name[NAME_MAX] = {0};

		const char *haystack = host;
		const char *needle = (const char *)":";
		const char *occurrence_COLON = strstr(haystack, needle);
		LOGGING(debug, "occurrence_COLON(:) = %p\n", occurrence_COLON);
		if (occurrence_COLON == NULL) {
			if (strlen(host) < NAME_MAX) {
				strncpy(name, host, strlen(host));
			} else {
				ret = -1;
				LOGGING(error, "Too long DOMAIN. %s\n", host);
				break;
			}
		} else {
			size_t n = occurrence_COLON - host;
			if (n < NAME_MAX) {
				strncpy(name, host, occurrence_COLON - host);
			} else {
				ret = -1;
				LOGGING(error, "Too long DOMAIN. %s\n", host);
				break;
			}
			const char *nptr = occurrence_COLON + 1;
			port = (uint16_t)atoi(nptr);
		}

		struct hostent *ht = NULL;
		ht = gethostbyname(name);
		if (ht == NULL) {
			LOGGING(error, "%s(%d)\n", hstrerror(h_errno), h_errno);
		        break;
		} else {
			LOGGING(debug, "ht = %p\n", ht);
		}
		
		char dst[NAME_MAX] = {0};
		socklen_t slt = sizeof dst;

		// On success, inet_ntop() returns a non-null pointer to dst.  NULL is returned if there was an error, with errno set to indicate the error.
		inet_ntop(domain, (void *)*(ht->h_addr_list), dst, slt);
		if (dst == NULL) {
			ret = -1;
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
		        break;
		} else {
			LOGGING(debug, "dst = %s, port = %u\n", dst, port);
		}

		struct sockaddr_in peer_addr;
		socklen_t addrlen = sizeof (struct sockaddr_in);

		memset(&peer_addr, 0, sizeof (struct sockaddr_in));
		peer_addr.sin_family = domain;
		peer_addr.sin_port = htons(port);
		ret = inet_pton(domain, dst, (struct sockaddr *) &peer_addr.sin_addr.s_addr);
		if (ret != 1) {
		        ret = -1;
		        LOGGING(error, "%s(%d)\n", strerror(errno), errno);
		        break;
		}

		ret = connect(connect_sock, (struct sockaddr *)&peer_addr, addrlen);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}


		// time_t created = time(NULL);
		LOGGING(notice, "It creates a new connected socket = %d.\n", connect_sock);
		ret = setnonblocking(connect_sock); /* Set Non Blocking */
		if (ret == -1) {
			break;
		}

		ev.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
		ev.data.fd = connect_sock;
		ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		char peer_ip[3 + 1 + 3 + 1 + 3 + 1 + 3 + 1] = {0};
		strncpy(peer_ip, inet_ntoa(peer_addr.sin_addr), sizeof peer_ip - 1);
		LOGGING(notice, "NAME ?:?->%s:%u\n", peer_ip, htons(peer_addr.sin_port));

		__t->set_fd(connect_sock);
		m->insert(std::make_pair(connect_sock, __t));

		w->push_back(__t);
#if 0
		const void *buf = __t->get_wx();
		size_t count = __t->get_ws();
		ssize_t rets = write(connect_sock, buf, count);
		if (rets == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}
		LOGGING(notice, "buf[%ld] = %s\n", rets, (char*)buf);


		int nfds = connect_sock + 1;
		fd_set readfds;
		fd_set writefds;
		fd_set exceptfds;
		struct timeval timeout;

		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
		FD_SET(connect_sock, &readfds);

		do {
			timeout.tv_sec = 3; /* seconds */

			ret = select(nfds, &readfds, &writefds, &exceptfds, &timeout);
			if (ret == -1) {
				LOGGING(error, "%s(%d)\n", strerror(errno), errno);
				break;
			} else if (ret == 0) {
				LOGGING(error, "timeout\n");
			} else if (ret > 0) {
				LOGGING(debug, "FD_ISSET(%d, %p) = %d\n", connect_sock, &readfds, FD_ISSET(connect_sock, &readfds));


				char rbuf[BUFFER_MAX] = {0};
				size_t rcount = BUFFER_MAX;
				rets = read(connect_sock, rbuf, rcount);

				LOGGING(notice, "rbuf[%ld] = %s\n", rets, rbuf);
				memset(rbuf, 0, rets);

				if (rets == -1) {
					LOGGING(error, "%s(%d)\n", strerror(errno), errno);
				} else if ((size_t)rets == rcount) {
					;
				} else if ((size_t)rets == 0) {
					break; // Closed
				}

			}

			if (quit) {
				break;
			}
		} while (true);
#endif

	} while (false);

	return ret;
}

