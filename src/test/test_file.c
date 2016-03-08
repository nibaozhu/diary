#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv) {

	void *buf = malloc(100);
	size_t count = 6;

	int fd, fd2;
	int r;

	const char *pathname = "/dev/pts/0";
	int flags = O_RDWR;

	fd = open(pathname, flags);
	if (fd == -1) {
		printf("open %s(%d)\n", strerror(errno), errno);
	}

	fd2 = dup(fd);

	/* set fd non-block */
	r = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (r == -1) {
		printf("fcntl %s(%d)\n", strerror(errno), errno);
	}

	while (1) {
		r = read(fd, buf, count);
		if (r == -1) {
			printf("read %s(%d), fd = %d\n", strerror(errno), errno, fd);
		}

		r = read(fd2, buf, count);
		if (r == -1) {
			printf("read %s(%d), fd2 = %d\n", strerror(errno), errno, fd2);
		}

		sleep(1);
	}

	return 0;
}
