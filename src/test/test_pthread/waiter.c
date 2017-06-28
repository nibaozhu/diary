#include "waiter.h"


void *waiter(void *arg) {
	LOGGING(debug, "Entering ...\n");

	personal_information_t *personal_information = (personal_information_t*)arg;

	if (personal_information != NULL) {
		LOGGING(notice, "personal_information: %p { department_ID: %d, employee_ID: %d }\n", 
				personal_information,
				personal_information->department_ID,
				personal_information->employee_ID
			);

		size_t i = 0;
		task_t *task;
		SLIST_FOREACH(task, personal_information->task_slist, entry) {

			task->ppid = getppid();
			task->pid = getpid();
			task->tid = syscall(SYS_gettid);
			task->ptid = pthread_self();

			LOGGING(notice, "task[%d]: { UUID: '%s', ID: %d, path: '%s', ppid: %d, pid: %d, tid: %d, ptid: 0x%lx }\n", 
					i++,
#ifdef UUID_LEN_STR
					task->UUID,
#else
					"(Not defined)",
#endif
					task->ID,
					task->path,
					task->ppid,
					task->pid,
					task->tid, 
					task->ptid
				);
		}
	}

	/* begin: do ... */

	int i;
	unsigned int un = 1; // UINT32_MAX;
	sleep(un);

	int n = INT32_MAX;
	for(i = 0; i < n; i++) {
		LOGGING(debug, "do something ...\n");
		sleep(1);
	}
	/* end: do ... */

	free(arg);

	LOGGING(debug, "Leaving\n");
	return NULL;
}
