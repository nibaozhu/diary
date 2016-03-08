#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(void) {

	const char *buf = "hello,world.\n";
	size_t count = strlen(buf);

	int i1 = 2;
	scanf("%d", &i1);
	printf("%d\n", i1);
	write(STDERR_FILENO, buf, count);
	write(STDERR_FILENO, buf, count);

	/* set stderr non-block */
	int r = fcntl(STDERR_FILENO, F_SETFL, O_NONBLOCK);
	if (r == -1) {
		printf("%s(%d)\n", strerror(errno), errno);
	}

	int ret;
	ret = scanf("%d", &i1);
	if (ret != 1) {
		printf("%s(%d)\n", strerror(errno), errno);
	}

	while (1) {
		write(STDERR_FILENO, buf, count);
		printf("%d\n", i1);
		sleep(1);
	}

	return 0;
}
