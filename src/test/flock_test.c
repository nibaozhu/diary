#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main()
{
	const char *pathname = "/tmp/file1";
	int flags = O_WRONLY|O_CREAT;
	int fd = open(pathname, flags);
	perror("open");

	int operation = LOCK_EX|LOCK_NB;
	int ret = flock(fd, operation);
	perror("flock");

	sleep(60*60*24); // 1 day
	close(fd);
	perror("close");

	return 0;
}
