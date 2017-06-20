#include "waiter.h"


void *waiter(void *arg) {
	plog(debug, "Entering ...\n");

	// pid_t pid = getpid();
	pid_t tid = syscall(SYS_gettid);
	pthread_t thread = pthread_self();

	plog(info, "[Thread 0x%lx (LWP %d)]\n", thread, tid);




	sleep(10);

	int i;
	for(i = 0; i < 1024; i++)
		plog(debug, "do something ...\n");


	plog(info, "[Thread 0x%lx (LWP %d)]\n", thread, tid);

	plog(debug, "Leaving\n");
	return NULL;
}
