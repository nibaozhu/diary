#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


void *start_routine(void *arg) {
	pid_t pid = getpid();
	pthread_t ptid = pthread_self();
	printf("[%s], process_id = %u (0x%x), thread_id = %u (0x%x)\n", (char*)arg, pid, pid, ptid, ptid);
	return arg;
}

int main() {
	int ret = 0;

	/* int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg); */
	pthread_t thread;
	const pthread_attr_t *attr = NULL;
	void *arg;

	arg = malloc(4);
	memset(arg, 0, sizeof arg);
	memcpy(arg, "YES", sizeof arg);

	pid_t pid = getpid();
	pthread_t ptid = pthread_self();
	printf("[%s], process_id = %u (0x%x), thread_id = %u (0x%x)\n", (char*)arg, pid, pid, ptid, ptid);

	ret = pthread_create(&thread, attr, start_routine, arg);
	if (ret != 0) {
		printf("%s(%d)\n", strerror(ret), ret);
		return ret;
	}

	/* int pthread_join(pthread_t thread, void **retval); */
	ret = pthread_join(thread, &arg);
	if (ret != 0) {
		printf("%s(%d)\n", strerror(ret), ret);
		return ret;
	}

	// sleep(3);

	return ret;
}
