/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <cassert>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <list>
#include <map>
#include <string>
#include <utility>

#include <arpa/inet.h>
#include "logging.h"

/* output BYTES bytes per output line */
#define WIDTH (1<<3)

/* 1024 bytes = 1KB */
#define SIZE (1<<10)

#define LENGTH (1<<2)

class Transport {
private:
	uint32_t id; /* unsigned int 32 */
	time_t created; /* the first communication time */
	time_t updated; /* the lastest communication time */
	bool alive; /* true: live; false: die */
	int fd; /* file descriptor */
	void *rx; /* transport data `Read'  */
	void *wx; /* transport data `Write' */
	size_t rp; /* transport data `Read' pointer position */
	size_t wp; /* transport data `Write' pointer position */
	size_t rs; /* transport data `Read' size  */
	size_t ws; /* transport data `Write' size */
	double speed; /* bytes per second */

	struct sockaddr_in peer_addr;
	socklen_t peer_addrlen;

public:
	Transport(int fd, time_t created, struct sockaddr_in peer_addr, socklen_t peer_addrlen, size_t size = SIZE) {
		assert(size > 0);

		this->created = created;
		this->updated = this->created;

		this->fd = fd;
		this->peer_addr = peer_addr;
		this->peer_addrlen = peer_addrlen;

		this->rp = 0;
		this->wp = 0;
		this->rs = size;
		this->ws = size;

		this->rx = malloc(this->rs);
		assert(this->rx != NULL);

		this->wx = malloc(this->ws);
		assert(this->wx != NULL);

		memset(this->rx, 0, this->rs);
		memset(this->wx, 0, this->ws);

		plog(debug, "new this = %p, malloc rx = %p, rp = 0x%lx, rs = 0x%lx, malloc wx = %p, wp = 0x%lx, ws = 0x%lx\n", 
				this, this->rx, this->rp, this->rs, this->wx, this->wp, this->ws);
	}

	uint32_t set_id(uint32_t id) {
		this->created = time(NULL);
		return this->id = id;
	}

	uint32_t get_id() {
		return this->id;
	}

	int set_fd(int fd) {
		return this->fd = fd;
	}

	int get_fd() {
		return this->fd;
	}

	size_t set_rp(size_t rp) {
		return this->rp = rp;
	}

	size_t get_rp() {
		return this->rp;
	}

	size_t set_wp(size_t wp) {
		return this->wp = wp;
	}

	size_t get_wp() {
		return this->wp;
	}

	void *set_rx(void *rx, size_t rs) {
		while (rs >= this->rs - this->rp) {
			plog(debug, "{rx = %p, rp = 0x%lx, rs = 0x%lx}, {0x%lx}\n", this->rx, this->rp, this->rs, rs);
			assert(this->rs > 0);
			this->rx = realloc(this->rx, this->rs << 1);
			if (this->rx == NULL) {
				plog(critical, "%s(%d)\n", strerror(errno), errno);
				return this->rx;
			} else {
				this->rs <<= 1;
				memset((void *)((char *)this->rx + this->rp), 0, this->rs - this->rp);
			}
		}
		memcpy((void *)((char *)this->rx + this->rp), rx, rs);
		this->rp += rs;
		this->updated = time(NULL);
		return (void *)((char *)this->rx + this->rp);
	}

	void *clear_rx(size_t size = SIZE) {
		assert(size > 0);
		memset(this->rx, 0, sizeof this->rp);
		this->rx = realloc(this->rx, size);
		this->rp = 0;
		this->rs = size;
		this->updated = time(NULL);
		return this->rx;
	}

	void *get_rx() {
		return this->rx;
	}

	void *set_wx(void *wx, size_t ws) {
		while (ws >= this->ws - this->wp) {
			plog(debug, "{wx = %p, wp = 0x%lx, ws = 0x%lx}, {0x%lx}\n", this->wx, this->wp, this->ws, ws);
			assert(this->ws > 0);
			this->wx = realloc(this->wx, this->ws << 1);
			if (this->wx == NULL) {
				plog(critical, "%s(%d)\n", strerror(errno), errno);
				return this->wx;
			} else {
				this->ws <<= 1;
				memset((void *)((char *)this->wx + this->wp), 0, this->ws - this->wp);
			}
		}
		memcpy((void *)((char *)this->wx + this->wp), wx, ws);
		this->wp += ws;
		this->updated = time(NULL);
		return (void *)((char *)this->wx + this->wp);
	}

	void *clear_wx(size_t size = SIZE) {
		memset(this->wx, 0, sizeof this->wp);
		assert(size > 0);
		this->wx = realloc(this->wx, size);
		assert(this->wx != NULL);
		this->wp = 0;
		this->ws = size;
		this->updated = time(NULL);
		return this->wx;
	}

	void *get_wx() {
		return this->wx;
	}

	size_t set_rs(size_t rs) {
		return this->rs = rs;
	}

	size_t get_rs() {
		return this->rs;
	}

	size_t set_ws(size_t ws) {
		return this->ws = ws;
	}

	size_t get_ws() {
		return this->ws;
	}

	bool set_alive(bool alive) {
		return this->alive = alive;
	}

	bool get_alive() {
		return this->alive;
	}

	double set_speed(double speed) {
		return this->speed = speed;
	}

	double get_speed() {
		return this->speed;
	}

	void pr(size_t width = WIDTH, bool b0 = false) {
		if (width <= 0 || width > 1024) {
			width = WIDTH;
		}
		assert(b0 == false);
		plog(info, "this = %p, this->rx = %p, this->rp = 0x%lx, this->rs = 0x%lx.\n", this, this->rx, this->rp, this->rs);
#if 0
		plog(info, "this = %p, this->rx = %p, this->rp = 0x%lx, this->rs = 0x%lx, b0 = %d\n", this, this->rx, this->rp, this->rs, b0);
		size_t i = 0;
		plog(debug, "--- begin (hexadecimal 2-byte units) -- %s --\n", __func__);
		while (i < this->rp) {
			if (i % width == 0) {
				plog(debug, "%p ", (void *)((char *)this->rx + i));
			}
			plog(debug, " 0x%02x", *((char*)this->rx + i));
			if (b0) {
				plog(debug, " %c", *((char*)this->rx + i));
			}

			i++;
			if (i % width == 0) {
				puts("");
			}
		}
		puts("\n--- end ---");
#endif
		return ;
	}

	void pw(size_t width = WIDTH, bool b0 = false) {
		if (width <= 0 || width > 1024) {
			width = WIDTH;
		}
		assert(b0 == false);
		plog(info, "this = %p, this->wx = %p, this->wp = 0x%lx, this->ws = 0x%lx.\n", this, this->wx, this->wp, this->ws);
#if 0
		plog(info, "this = %p, this->wx = %p, this->wp = 0x%lx, this->ws = 0x%lx, b0 = %d\n", this, this->wx, this->wp, this->ws, b0);
		size_t i = 0;
		plog(debug, "--- begin (hexadecimal 2-byte units) -- %s --\n", __func__);
		while (i < this->wp) {
			if (i % width == 0) {
				plog(debug, "%p ", (void *)((char *)this->wx + i));
			}
			plog(debug, " 0x%02x", *((char*)this->wx + i));
			if (b0) {
				plog(debug, " %c", *((char*)this->rx + i));
			}

			i++;
			if (i % width == 0) {
				puts("");
			}
		}
		puts("\n--- end ---");
#endif
		return ;
	}

	~Transport() {
		plog(debug, "delete this = %p, free rx = %p, rp = 0x%lx, rs = 0x%lx, free wx = %p, wp = 0x%lx, ws = 0x%lx\n", 
				this, this->rx, this->rp, this->rs, this->wx, this->wp, this->ws);
		free(this->rx);
		free(this->wx);
	}
};

#endif
