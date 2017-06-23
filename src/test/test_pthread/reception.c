#include "reception.h"


void *reception(void *arg) {
	plog(debug, "Entering ...\n");

	personal_information_t *personal_information = (personal_information_t*)arg;

	if (personal_information != NULL) {
		plog(notice, "personal_information: %p { department_ID: %d, employee_ID: %d }\n", 
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

			plog(notice, "task[%d]: { UUID: '%s', ID: %d, path: '%s', ppid: %d, pid: %d, tid: %d, ptid: 0x%lx }\n", 
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


	sleep(3);

	int i;
	for(i = 0; i < 2000000; i++)
		plog(debug, "do something ...\n");

	free(arg);

	plog(debug, "Leaving\n");
	return NULL;
}
