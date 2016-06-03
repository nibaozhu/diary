/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "client.h"
#include "task.h"

extern struct epoll_event ev, events[MAX_EVENTS];
extern int listen_sock, nfds, epollfd;
extern bool quit;

int connect_to_host(const char *host, uint16_t port, const char *buf, size_t count) {
	int ret = 0;

	int domain = AF_INET;
	int type = SOCK_STREAM;
	int protocol = 0;
	int connect_sock = 0;

	do {
		connect_sock = socket(domain, type, protocol);
		if (connect_sock < 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		char name[NAME_MAX] = {0};

		const char *haystack = host;
		const char *needle = (const char *)":";
		const char *occurrence_COLON = strstr(haystack, needle);
		plog(debug, "occurrence_COLON(:) = %p\n", occurrence_COLON);
		if (occurrence_COLON == NULL) {
			if (strlen(host) < NAME_MAX) {
				strncpy(name, host, strlen(host));
			} else {
				ret = -1;
				plog(error, "Too long DOMAIN. %s\n", host);
				break;
			}
		} else {
			size_t n = occurrence_COLON - host;
			if (n < NAME_MAX) {
				strncpy(name, host, occurrence_COLON - host);
			} else {
				ret = -1;
				plog(error, "Too long DOMAIN. %s\n", host);
				break;
			}
			const char *nptr = occurrence_COLON + 1;
			port = (uint16_t)atoi(nptr);
		}

		struct hostent *ht = NULL;
		ht = gethostbyname(name);
		if (ht == NULL) {
			plog(error, "%s(%d)\n", hstrerror(h_errno), h_errno);
		        break;
		} else {
			plog(debug, "ht = %p\n", ht);
		}
		
		char dst[NAME_MAX] = {0};
		socklen_t slt = sizeof dst;

		// On success, inet_ntop() returns a non-null pointer to dst.  NULL is returned if there was an error, with errno set to indicate the error.
		inet_ntop(domain, (void *)*(ht->h_addr_list), dst, slt);
		if (dst == NULL) {
			ret = -1;
			plog(error, "%s(%d)\n", strerror(errno), errno);
		        break;
		} else {
			plog(debug, "dst = %s, port = %u\n", dst, port);
		}

		struct sockaddr_in addr;
		socklen_t addrlen = sizeof (struct sockaddr_in);

		memset(&addr, 0, sizeof (struct sockaddr_in));
		addr.sin_family = domain;
		addr.sin_port = htons(port);
		ret = inet_pton(domain, dst, (struct sockaddr *) &addr.sin_addr.s_addr);
		if (ret != 1) {
		        ret = -1;
		        plog(error, "%s(%d)\n", strerror(errno), errno);
		        break;
		}

		ret = connect(connect_sock, (struct sockaddr *)&addr, addrlen);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		plog(notice, "It creates a new connected socket = %d.\n", connect_sock);
		ret = setnonblocking(connect_sock); /* Set Non Blocking */
		if (ret == -1) {
			break;
		}


		ssize_t rets = write(connect_sock, buf, count);
		if (rets == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}
		plog(notice, "buf[%ld] = %s\n", rets, buf);


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
				plog(error, "%s(%d)\n", strerror(errno), errno);
				break;
			} else if (ret == 0) {
				plog(error, "timeout\n");
			} else if (ret > 0) {
				plog(debug, "FD_ISSET(%d, %p) = %d\n", connect_sock, &readfds, FD_ISSET(connect_sock, &readfds));


				char rbuf[BUFFER_MAX] = {0};
				size_t rcount = BUFFER_MAX;
				rets = read(connect_sock, rbuf, rcount);

				plog(notice, "rbuf[%ld] = %s\n", rets, rbuf);
				memset(rbuf, 0, rets);

				if (rets == -1) {
					plog(error, "%s(%d)\n", strerror(errno), errno);
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

	} while (false);

	return ret;
}
