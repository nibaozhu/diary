#include "reception.h"


void *reception(void *arg) {
	syslog(LOG_DEBUG, "Entering ...\n");

	personal_information_t *personal_information = 
		(personal_information_t*)arg;

	if (personal_information != NULL) {
		syslog(LOG_NOTICE, "personal_information:%p "
			"{ department_ID:%lu, employee_ID:%lu }\n", 
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

			syslog(LOG_NOTICE, "task[%lu]: "
				"{ UUID:'%s', ID:%lu, path:'%s', "
				"ppid:%d, pid:%d, tid:%d, ptid: 0x%lx }\n", 
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
	int i, n = INT32_MAX;
	for(i = 0; i < n; i++) {
		int type = rand() % (LOG_DEBUG) + 1; // avoid LOG_EMERG
		int timeout = rand() % (int)(1e6);

		syslog(type, "do something ...\n");
		usleep(timeout);
		syslog(type, "type:%d, done <%d microseconds>\n", type, timeout);
		if(hotel.bankruptcy) break;
	}
	/* end: do ... */

	free(arg);

	syslog(LOG_DEBUG, "Leaving\n");
	return NULL;
}
