#include "worker.h"


void *worker(void *arg) {
	syslog(LOG_DEBUG, "Entering ...\n");

	syslog(LOG_NOTICE, "arg: %p", arg);
	if (arg == NULL) { return NULL; }
	info_t *info = (info_t*)arg;

	info->task_slist = (struct task_slist_s*)malloc(sizeof(struct task_slist_s));
	SLIST_INIT(info->task_slist);

	// int j;
	// for (j = 0; j < chatd.number; j++) {
	// 	task_t *task = (task_t*)malloc(sizeof(task_t));
	// 	if (task == NULL) return EXIT_FAILURE;

	// 	/* Generate a task. */
	// 	task->ID = random();
	// 	strncpy(task->UUID, "xxx", UUID_LEN_STR);

	// 	SLIST_INSERT_HEAD(info->task_slist, task, entry);
	// }

	size_t i = 0;
	task_t *task;
	SLIST_FOREACH(task, info->task_slist, entry) {
		task->ppid = getppid();
		task->pid = getpid();
		task->tid = syscall(SYS_gettid);
		task->ptid = pthread_self();
		syslog(LOG_NOTICE, "task[%lu]: {UUID:'%s', ID:%lu, ppid:%d, pid:%d, tid:%d, ptid: 0x%lx}\n", 
			i++, task->UUID, task->ID, task->ppid, task->pid, task->tid, task->ptid);
	}

	/* begin: do ... */
	size_t n = INT32_MAX;
	for(i = 0; i < n; i++) {
		int type = rand() % (LOG_DEBUG) + 1; // avoid LOG_EMERG
		int timeout = rand() % (int)(1e6);

		syslog(type, "do something ...\n");
		usleep(timeout);
		syslog(type, "type:%d, done <%d microseconds>\n", type, timeout);
		if(chatd.bankruptcy) break;
	}
	/* end: do ... */

	free(arg);
	syslog(LOG_DEBUG, "Leaving\n");
	return NULL;
}
