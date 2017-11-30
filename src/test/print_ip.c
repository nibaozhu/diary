#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/limits.h>

int main(int argc, char **argv) {
	char name[NAME_MAX + 1] = { 0 };
	size_t len = NAME_MAX;
	int ret = 0;

	do {
		ret = gethostname(name, len);
		if(ret != 0) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			break;
		} else {
			fprintf(stdout, "name: '%s'\n", name);
		}

		const char *node = name;
		const char *service = NULL;
		struct addrinfo hints, *hptr = NULL, *res = NULL;
		memset(&hints, 0, sizeof (struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		int errcode = getaddrinfo(node, service, (const struct addrinfo *) &hints, &res);
		if(errcode != 0) {
			fprintf(stderr, "%s(%d)\n", gai_strerror(errcode), errcode);
			break;
		} else {
			fprintf(stdout, "res: %p\n", res);
		}

		int af = AF_INET;
		const void *src = NULL;
		char dst[NAME_MAX + 1] = { 0 };
		socklen_t size = NAME_MAX;
		for(hptr = res; hptr != NULL; hptr = hptr->ai_next) {
			src = &((struct sockaddr_in *)(hptr->ai_addr))->sin_addr;
			const char *pret = inet_ntop(af, src, dst, size);
			if(pret == NULL) {
				fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
				break;
			} else {
				fprintf(stdout, "dst: '%s'\n", dst);
			}
		}
	} while(0);
	return 0;
}
