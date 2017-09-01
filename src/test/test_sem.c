#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#include <string.h>
#include "logging.h"


int main(int argc, char **argv) {
	/* (1): Begin ... */
	initializing(argv[0], "/tmp", "w+", debug, debug, LOGGING_INTERVAL, 0, LOGGING_SIZE);

	assert(argc > 1);
	char name[NAME_MAX-4+1] = { 0 };
	strncpy(name, argv[1], NAME_MAX-4);

	int oflag = O_CREAT; // | O_EXCL;
	mode_t mode = 0644;
	unsigned int value = 0;

	sem_t *sem = sem_open(name, oflag, mode, value);
	if (sem == SEM_FAILED) {
		LOGGING(error, "%s(%d)\n", strerror(errno), errno);
		exit(1);
	}

#ifdef SEM_WAIT
	while (true) {
		int ret = sem_wait(sem);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
		}

		int sval = 0;
		ret = sem_getvalue(sem, &sval);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
		}

		LOGGING(debug, "sem_wait() decrements (locks) the semaphore pointed to by "
			"sem = %p, sval = %d\n", sem, sval);
		// sleep(1);
	}
#endif

#ifdef SEM_POST
	while (true) {
		int ret = sem_post(sem);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
		}

		int sval = 0;
		ret = sem_getvalue(sem, &sval);
		if (ret == -1) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
		}

		LOGGING(debug, "sem_post() increments (unlocks) the semaphore pointed to by "
			"sem = %p, sval = %d\n", sem, sval);
		sleep(1);
	}
#endif

	/* (3): End ... */
	return uninitialized();
}
