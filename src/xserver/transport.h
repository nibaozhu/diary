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
#include <sys/types.h>

#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>

#include <logging.h>

/* output BYTES bytes per line */
#define WIDTH (1<<3)

/* 1024 bytes = 1KB */
#define SIZE (1<<10)

#define BUFFER_MAX (1<<16)

class AppChatProtocol {
public:
	uint32_t length = 0;
	uint32_t crc32 = 0;
	uint32_t magic = 0;

	void *body;
};

class Transport {
	public:
		std::string appid; /* AppChat Unique ID */
		std::string dtime;
		std::string sender;
		std::string passwd;
		std::string receiver;
		std::string context;
		std::string command; /* AppChatProtocol Command */
		std::string errcode;
		std::string errstring;

	private:
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

		struct sockaddr_in peer_addr;
		socklen_t peer_addrlen;
		__uint32_t events;

	public:
		Transport(int fd, time_t created, struct sockaddr_in peer_addr, socklen_t peer_addrlen, size_t size = SIZE) {
			assert(size > 0);

			this->updated = this->created = created;

			this->appid = "";
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

			this->set_alive(true);
			this->set_events(0);

			LOGGING(debug, "Construct %p. malloc rx = %p, rp = 0x%lx, rs = 0x%lx, malloc wx = %p, wp = 0x%lx, ws = 0x%lx\n", 
					this, this->rx, this->rp, this->rs, this->wx, this->wp, this->ws);
		}

		time_t set_updated() {
			return this->updated = time(NULL);
		}

		time_t get_updated() {
			return this->updated;
		}

		bool set_alive(bool alive) {
			this->set_updated();
			return this->alive = alive;
		}

		bool get_alive() {
			return this->alive;
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
				LOGGING(debug, "realloc {rx = %p, rp = 0x%lx, rs = 0x%lx}, {0x%lx}\n", this->rx, this->rp, this->rs, rs);
				assert(this->rs > 0);
				void *tx = realloc(this->rx, this->rs << 1);
				if (tx == NULL) {
					LOGGING(critical, "%s(%d)\n", strerror(errno), errno);
					return tx;
				} else {
					this->rx = tx;
					this->rs <<= 1;
					memset((void *)((char *)this->rx + this->rp), 0, this->rs - this->rp);
				}
			}
			memcpy((void *)((char *)this->rx + this->rp), rx, rs);
			this->rp += rs;
			this->set_updated();
			return (void *)((char *)this->rx + this->rp);
		}

		void *clear_rx(size_t size = SIZE) {
			assert(size > 0);
			memset(this->rx, 0, sizeof this->rp);
			void *tx = realloc(this->rx, size);
			if (tx == NULL) {
				LOGGING(critical, "%s(%d)\n", strerror(errno), errno);
				return tx;
			} else {
				this->rx = tx;
			}

			this->rp = 0;
			this->rs = size;
			this->set_updated();
			return this->rx;
		}

		void *get_rx() {
			return this->rx;
		}

		void *set_wx(void *wx, size_t ws) {
			while (ws >= this->ws - this->wp) {
				LOGGING(debug, "realloc {wx = %p, wp = 0x%lx, ws = 0x%lx}, {0x%lx}\n", this->wx, this->wp, this->ws, ws);
				assert(this->ws > 0);
				void *tx = realloc(this->wx, this->ws << 1);
				if (tx == NULL) {
					LOGGING(critical, "%s(%d)\n", strerror(errno), errno);
					return tx;
				} else {
					this->wx = tx;
					this->ws <<= 1;
					memset((void *)((char *)this->wx + this->wp), 0, this->ws - this->wp);
				}
			}
			memcpy((void *)((char *)this->wx + this->wp), wx, ws);
			this->wp += ws;
			this->set_updated();
			return (void *)((char *)this->wx + this->wp);
		}

		void *clear_wx(size_t size = SIZE) {
			memset(this->wx, 0, sizeof this->wp);
			assert(size > 0);
			void *tx = realloc(this->wx, size);
			if (tx == NULL) {
				LOGGING(critical, "%s(%d)\n", strerror(errno), errno);
				return tx;
			} else {
				this->wx = tx;
			}

			assert(this->wx != NULL);
			this->wp = 0;
			this->ws = size;
			this->set_updated();
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

		struct sockaddr_in set_peer(struct sockaddr_in peer_addr, socklen_t peer_addrlen) {
			this->peer_addr = peer_addr;
			this->peer_addrlen = peer_addrlen;
			return this->peer_addr;
		}

		struct sockaddr_in get_peer(struct sockaddr_in *peer_addr, socklen_t *peer_addrlen) {
			assert(peer_addr != NULL && peer_addrlen != NULL);
			*peer_addr = this->peer_addr;
			*peer_addrlen = this->peer_addrlen;
			return *peer_addr;
		}

		__uint32_t set_events(__uint32_t events) {
			return this->events= events;
		}

		__uint32_t get_events() {
			return this->events;
		}

		void printread(size_t width = WIDTH) {
			if (width <= 0 || width > 1024) {
				width = WIDTH;
			}

			struct sockaddr_in peer_addr;
			socklen_t peer_addrlen;
			this->get_peer(&peer_addr, &peer_addrlen);

			char peer_ip[16];
			strcpy(peer_ip, inet_ntoa(peer_addr.sin_addr));

			LOGGING(info, "|%s:%u| this = %p, this->rx = %p, this->rp = 0x%lx, this->rs = 0x%lx\n",
					peer_ip, htons(peer_addr.sin_port), this, this->rx, this->rp, this->rs);

#ifndef NDEBUG
			size_t i = 0;
			LOGGING(debug, "--- begin (hexadecimal 2-byte units) -- %s --\n", __func__);
			while (i < this->rp) {
				if (i % width == 0) {
					LOGGING(debug, "%p ", (void *)((char *)this->rx + i));
				}
				LOGGING(debug, " 0x%02x", *((char*)this->rx + i));

				i++;
				if (i % width == 0) {
					puts("");
				}
			}
			puts("\n--- end ---");
#endif
			return ;
		}

		void printwrite(size_t width = WIDTH) {
			if (width <= 0 || width > 1024) {
				width = WIDTH;
			}

			struct sockaddr_in peer_addr;
			socklen_t peer_addrlen;
			this->get_peer(&peer_addr, &peer_addrlen);

			char peer_ip[16];
			strcpy(peer_ip, inet_ntoa(peer_addr.sin_addr));

			LOGGING(info, "|%s:%u| this = %p, this->wx = %p, this->wp = 0x%lx, this->ws = 0x%lx\n",
					peer_ip, htons(peer_addr.sin_port), this, this->wx, this->wp, this->ws);

#ifndef NDEBUG
			size_t i = 0;
			LOGGING(debug, "--- begin (hexadecimal 2-byte units) -- %s --\n", __func__);
			while (i < this->wp) {
				if (i % width == 0) {
					LOGGING(debug, "%p ", (void *)((char *)this->wx + i));
				}
				LOGGING(debug, " 0x%02x", *((char*)this->wx + i));

				i++;
				if (i % width == 0) {
					puts("");
				}
			}
			puts("\n--- end ---");
#endif
			return ;
		}

		void reset() {
			this->clear_rx();
			this->clear_wx();
			this->set_updated();
			return ;
		}

		~Transport() {
			LOGGING(debug, "Deconstruct %p. free rx = %p, rp = 0x%lx, rs = 0x%lx, free wx = %p, wp = 0x%lx, ws = 0x%lx\n", 
					this, this->rx, this->rp, this->rs, this->wx, this->wp, this->ws);
			free(this->rx);
			free(this->wx);
		}
};

#endif
