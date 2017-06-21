#include "waiter.h"


void *waiter(void *arg) {
	plog(debug, "Entering ...\n");

	personal_information_t *personal_information = (personal_information_t*)arg;

	pid_t pid = getpid();
	(void)pid;

	pid_t tid = syscall(SYS_gettid);
	pthread_t thread = pthread_self();

	plog(info, "[Thread 0x%lx (LWP %d)] arg: %p\n", thread, tid, personal_information);

	if (personal_information != NULL) {
		plog(notice, "department_ID: %d, employee_ID: %d\n", 
				personal_information->department_ID,
				personal_information->employee_ID
			);
	}


	int i;
	for(i = 0; i < 10; i++)
		plog(debug, "do something ...\n");

	sleep(4);



	plog(info, "[Thread 0x%lx (LWP %d)] arg: %p\n", thread, tid, personal_information);

	free(arg);

	plog(debug, "Leaving\n");
	return NULL;
}