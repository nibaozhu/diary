#include "reception.h"


void *reception(void *arg) {
	plog(debug, "Entering ...\n");

	// pid_t pid = getpid();
	pid_t tid = syscall(SYS_gettid);
	pthread_t thread = pthread_self();

	plog(info, "[Thread 0x%lx (LWP %d)]\n", thread, tid);




	sleep(10);




	plog(info, "[Thread 0x%lx (LWP %d)]\n", thread, tid);

	plog(debug, "Leaving\n");
	return NULL;
}
