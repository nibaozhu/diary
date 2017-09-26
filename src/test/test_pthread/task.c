#include "task.h"


void *do_receive(void *arg) {
	syslog(LOG_DEBUG, "Entering ...\n");

	syslog(LOG_NOTICE, "arg: %p", arg);
	if (arg == NULL) { return NULL; }
	info_t *info = (info_t*)arg;

	// int j;
	// for (j = 0; j < chat.number; j++) {
	// 	task_t *task = (task_t*)malloc(sizeof(task_t));
	// 	if (task == NULL) return EXIT_FAILURE;

	// 	/* Generate a task. */
	// 	task->ID = random();
	// 	strncpy(task->UUID, "xxx", UUID_LEN_STR);

	// 	SLIST_INSERT_HEAD(info->task_slist, task, entry);
	// }

	while (true) {
		doing(info->task_slist);

		if (chat.bankruptcy) break;
	}

	free(arg);
	syslog(LOG_DEBUG, "Leaving\n");
	return NULL;
}

int doing(struct task_slist_s *task_slist) {
	size_t i = 0;
	task_t *task;
	SLIST_FOREACH(task, task_slist, entry) {
		task->ppid = getppid();
		task->pid = getpid();
		task->tid = syscall(SYS_gettid);
		task->ptid = pthread_self();
		syslog(LOG_NOTICE, "task[%lu]: {UUID:'%s', ID:%lu, ppid:%d, pid:%d, tid:%d, ptid: 0x%lx}\n", 
			i++, task->UUID, task->ID, task->ppid, task->pid, task->tid, task->ptid);
	}

	return 0;
}

