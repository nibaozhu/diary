#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <event.h>


void accept_fn(evutil_socket_t fd, short events, void *arg);
void bev_data_read_cb(struct bufferevent *bev, void *ctx);
void bev_data_write_cb(struct bufferevent *bev, void *ctx);


int main(int argc, char **argv) {

	int ret = 0;
	do {
		int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock < 0) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		const char *ip = "127.0.0.1";
		short int port = 12340;
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof (struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		ret = inet_pton(AF_INET, ip, (struct sockaddr *) &addr.sin_addr.s_addr);
		if (ret != 1) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		evutil_socket_t sock = listen_sock;
		ret = evutil_make_listen_socket_reuseable(sock);
		if (ret == -1) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			ret = -11;
			break;
		}

		socklen_t addrlen = sizeof (struct sockaddr_in);
		ret = bind(listen_sock, (struct sockaddr *) &addr, addrlen);
		if (ret == -1) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		int backlog = (1<<10);
		ret = listen(listen_sock, backlog);
		if (ret == -1) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		ret = evutil_make_socket_nonblocking(sock);
		if (ret == -1) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			ret = -12;
			break;
		}

		struct event_base *eb = event_base_new();
		if (eb == NULL) {
			ret = -2;
			fprintf(stderr, "eb: %p\n", eb);
			break;
		}
		fprintf(stdout, "eb: %p\n", eb);

		evutil_socket_t fd = listen_sock;
		short events = EV_READ|EV_PERSIST;
		event_callback_fn evfn = accept_fn;
		struct event *ev = event_new(eb, fd, events, evfn, (void *)eb);
		if (ev == NULL) {
			ret = -3;
			fprintf(stderr, "ev: %p\n", ev);
			break;
		}
		fprintf(stdout, "ev: %p\n", ev);

		const struct timeval *timeout = NULL;
		ret = event_add(ev, timeout);
		if (ret == -1) {
			ret = -9;
			fprintf(stderr, "ret: %d\n", ret);
			break;
		}
		fprintf(stdout, "ret: %d\n", ret);

		ret = event_base_dispatch(eb);
		if (ret == -1) {
			ret = -10;
			fprintf(stderr, "ret: %d\n", ret);
			break;
		}
		fprintf(stdout, "ret: %d\n", ret);

	} while (0);
	if (ret == -1) {
		return ret;
	}

	return 0;
}


void accept_fn(evutil_socket_t fd, short events, void *arg) {
	struct event_base *eb = (struct event_base *)arg;
	struct sockaddr_in addr;
	socklen_t addrlen;
	int ret = 0;

	evutil_socket_t afd = accept(fd, (struct sockaddr *)&addr, &addrlen);
	if (afd < 0) {
		fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
		return ;
	}
	fprintf(stdout, "afd: %d\n", afd);

	int options = BEV_OPT_CLOSE_ON_FREE;
	struct bufferevent *bufev = bufferevent_socket_new(eb, afd, options);
	if (bufev == NULL) {
		ret = -4;
		fprintf(stderr, "bufev: %p\n", bufev);
		return ;
	}
	fprintf(stdout, "bufev: %p\n", bufev);

	bufferevent_data_cb readcb = bev_data_read_cb;
	bufferevent_data_cb writecb = bev_data_write_cb;
	bufferevent_event_cb eventcb = NULL;
	void *cbarg = eb;
	bufferevent_setcb(bufev, readcb, writecb, eventcb, cbarg);

	short event = EV_READ | EV_WRITE;
	ret = bufferevent_enable(bufev, event);
	if (ret == -1) {
		ret = -8;
		fprintf(stderr, "ret: %d\n", ret);
		return ;
	}
}

void bev_data_read_cb(struct bufferevent *bev, void *ctx) {
	int ret = 0;
	evutil_socket_t fd = bufferevent_getfd(bev);
	if (fd == -1) {
		ret = -5;
		fprintf(stderr, "fd: %d\n", fd);
		return ;
	}
	fprintf(stdout, "fd: %d\n", fd);

	struct evbuffer *buf = evbuffer_new();
	if (buf == NULL) {
		ret = -6;
		fprintf(stdout, "ret: %d\n", ret);
		return ;
	}

	ret = bufferevent_read_buffer(bev, buf);
	if (ret == -1) {
		ret = -7;
		fprintf(stdout, "ret: %d\n", ret);
		return ;
	}

	char data[BUFSIZ];
	size_t datlen = BUFSIZ - 1;
	while (1) {
		static int i = 0;
		memset(&data, 0, sizeof data);
		int ss = evbuffer_remove(buf, data, datlen);
		fprintf(stdout, "data[%d]: %s\n", i++, data);
		if (ss < datlen) {
			break;
		}
	}

	evbuffer_free(buf);
	buf = NULL;
}


void bev_data_write_cb(struct bufferevent *bev, void *ctx) {
	;;;
}
