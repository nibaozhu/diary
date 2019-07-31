#include <cerrno>
#include <cstdio>
#include <cstring>

#include <event.h>
#include <event2/listener.h>

#include <arpa/inet.h>

#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>


void listener_cb(struct evconnlistener *lev, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *user_arg);
void bev_data_read_cb(struct bufferevent *bev, void *ctx);
void bev_data_write_cb(struct bufferevent *bev, void *ctx);
void bev_data_event_cb(struct bufferevent *bev, short what, void *ctx);


int main(int argc, char **argv) {
	int ret = 0;
	do {
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

		struct event_base *eb = event_base_new();
		if (eb == NULL) {
			ret = -2;
			fprintf(stderr, "eb: %p\n", eb);
			break;
		}
		fprintf(stdout, "eb: %p\n", eb);

		evconnlistener_cb listenercb = listener_cb;
		unsigned flags = LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE;
		int backlog = (1<<10);
		int socklen = sizeof (struct sockaddr_in);
		struct evconnlistener *lev = evconnlistener_new_bind(eb, listenercb, (void*)eb, 
										flags, backlog, (struct sockaddr *) &addr, socklen);
		if (lev == NULL) {
			ret = -14;
			fprintf(stderr, "lev: %p\n", lev);
			break;
		}
		fprintf(stdout, "lev: %p\n", lev);

		ret = event_base_dispatch(eb);
		if (ret == -1) {
			ret = -10;
			fprintf(stderr, "ret: %d\n", ret);
			break;
		}
		fprintf(stdout, "ret: %d\n", ret);

		event_base_free(eb);
		evconnlistener_free(lev);
	} while (0);
	if (ret == -1) {
		return ret;
	}

	return 0;
}

void listener_cb(struct evconnlistener *lev, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *user_arg) {
	struct event_base *eb = (struct event_base *)user_arg;
	int ret = 0;
	int options = BEV_OPT_CLOSE_ON_FREE;
	struct bufferevent *bufev = bufferevent_socket_new(eb, fd, options);
	if (bufev == NULL) {
		ret = -4;
		fprintf(stderr, "bufev: %p\n", bufev);
		return ;
	}
	fprintf(stdout, "bufev: %p\n", bufev);

	bufferevent_data_cb readcb = bev_data_read_cb;
	bufferevent_data_cb writecb = bev_data_write_cb;
	bufferevent_event_cb eventcb = bev_data_event_cb;
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
		if (ss == -1) {
			fprintf(stderr, "ss: %d\n", ss);
			break;
		} else if (ss == 0) {
			break;
		}

		fprintf(stdout, "data[%d]: %s\n", i, data);

		BIO *b64 = BIO_new(BIO_f_base64());
		BIO *bmem = BIO_new(BIO_s_mem());
		b64 = BIO_push(b64, bmem);
		BIO_write(b64, data, ss);
		(void) BIO_flush(b64);
		BUF_MEM *bptr = NULL;
		BIO_get_mem_ptr(b64, &bptr);
		fprintf(stdout, "data base64[%d]: %s\n", i, bptr->data);

		ret = bufferevent_write(bev, bptr->data, bptr->length);
		BIO_free_all(b64);
		if (ret == -1) {
			ret = -13;
			fprintf(stdout, "ret: %d\n", ret);
			break ;
		}

		if ((size_t)ss < datlen) {
			break;
		}
		i++;
	}

	evbuffer_free(buf);
	buf = NULL;
}

void bev_data_write_cb(struct bufferevent *bev, void *ctx) {
	int ret = 0;
	fprintf(stdout, "bev_data_write_cb: %d\n", ret);
}

void bev_data_event_cb(struct bufferevent *bev, short what, void *ctx) {
	fprintf(stdout, "bev_data_event_cb: bev: %p, what: 0x%x, ctx: %p\n", bev, what, ctx);

	if (what & BEV_EVENT_READING) {
		fprintf(stderr, "error encountered while reading\n");
	}

	if (what & BEV_EVENT_WRITING) {
		fprintf(stderr, "error encountered while writing\n");
	}

	if (what & BEV_EVENT_EOF) {
		fprintf(stderr, "eof file reached\n");
	}

	if (what & BEV_EVENT_ERROR) {
		fprintf(stderr, "unrecoverable error encountered\n");
	}

	if (what & BEV_EVENT_TIMEOUT) {
		fprintf(stderr, "user-specified timeout reached\n");
	}

	if (what & BEV_EVENT_CONNECTED) {
		fprintf(stderr, "connect operation finished.\n");
	}
}

