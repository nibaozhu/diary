/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "client.h"
#include "task.h"

extern struct epoll_event ev, events[MAX_EVENTS];
extern int listen_sock, nfds, epollfd;

int connect_to_host(const char *host, int &connect_sock) {
	int ret = 0;

	int domain = AF_INET;
	int type = SOCK_STREAM;
	int protocol = 0;

	do {
		connect_sock = socket(domain, type, protocol);
		if (connect_sock < 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			ret = -1;
			break;
		}

		char name[NAME_MAX] = {0};
		uint16_t port = 80;

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

#if 0
		time_t created = time(NULL);
		plog(notice, "It creates a new connected socket = %d.\n", connect_sock);
		ret = setnonblocking(connect_sock); /* Set Non Blocking */
		if (ret == -1) {
			break;
		}

		ev.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
		ev.data.fd = connect_sock;
		ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
		if (ret == -1) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			break;
		}

		plog(notice, "NAME %s:%u\n", dst, port);
		socklen_t dst_addrlen = sizeof (struct sockaddr_in);
		Transport* t = NULL;

		struct sockaddr_in peer_addr;
		socklen_t peer_addrlen = sizeof (struct sockaddr_in);
		memset(&peer_addr, 0, sizeof (struct sockaddr_in));

		t = new Transport(connect_sock, created, dst, dst_addrlen);
		m->insert(std::make_pair(connect_sock, t));
#endif

	} while (false);

	return ret;
}
