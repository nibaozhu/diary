#include "dog.h"


/* Set sub-thread ... */
dog_t dog = { .name = "Happiness", .number = 0x10, .bankruptcy = false };

void handler(int signum) {
	pid_t ppid, pid, tid;
	pthread_t ptid;

	syslog(LOG_NOTICE, "{ signum:%d, ppid:%d, pid:%d, tid:%d, ptid:0x%lx }\n", signum,
			ppid = getppid(), pid = getpid(), tid = syscall(SYS_gettid),
			ptid = pthread_self());

	switch (signum) {
		case SIGHUP :
			break;
		case SIGINT :
		case SIGQUIT:
		case SIGTERM:
		case SIGUSR1:
		case SIGUSR2:
			if (pid == tid) dog.bankruptcy = true; // FIXME: rwlock
			break;
		default: ; /* do nothing */
	}
}

void set_disposition(void) {
	size_t i, signum_arr[] = {SIGHUP, SIGINT, SIGQUIT, SIGTERM, 
					SIGUSR1, SIGUSR2};
	for (i = 0; i < sizeof (signum_arr) / sizeof (size_t); i++) {
		if (SIG_ERR == signal(signum_arr[i], handler)) {
			syslog(LOG_CRIT, "signum_arr[%lu]:%lu\n", i, signum_arr[i]);
			exit(signum_arr[i]);
		}
	}
}

int main(int argc, char **argv) {
	const char *ident = basename(argv[0]);
	int option = LOG_CONS | LOG_PID;
	int facility = LOG_USER;
	openlog(ident, option, facility);

	int r = 0;
	// switch (r = fork()) {
	// 	case -1: break;
	// 	case 0: setsid(); break;
	// 	default: exit(0);
	// }

	// if (r == -1) {
	// 	syslog(LOG_CRIT, "%s(%d)\n", strerror(errno), errno);
	// 	exit(0);
	// }

	set_disposition();
	dog.pthread = 
		(pthread_t *)malloc(dog.number * sizeof(pthread_t));
	if (dog.pthread == NULL) return EXIT_FAILURE;

	pthread_attr_t *attr = 
		(pthread_attr_t *)malloc(sizeof(pthread_attr_t));
	if (attr == NULL) return EXIT_FAILURE;

	r = pthread_attr_init(attr);
	if (r != 0) {
		syslog(LOG_CRIT, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	size_t i;
	for (i = 0; i < dog.number; i++) {

		/* info_t will be freed at sub-thread. */
		info_t *info = (info_t*)malloc(sizeof(info_t));
		info->ID = i;
		info->task_slist = (struct task_slist_s*)malloc(sizeof(struct task_slist_s));
		SLIST_INIT(info->task_slist);

		void *arg = info;
		void *(*start_routine) (void *) = do_task;

		r = pthread_create(dog.pthread + i, (const pthread_attr_t *)attr,
						start_routine, arg);
		if (r != 0) {
			syslog(LOG_ERR, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		/* XXX: `arg' maybe had been freed, and we just look it. */
		syslog(LOG_NOTICE, "created: Thread[%lu]:0x%lx, arg:%p\n", 
			i, *(dog.pthread + i), arg);
	}

	r = pthread_attr_destroy(attr);
	if (r != 0) {
		syslog(LOG_CRIT, "%s(%d)\n", strerror(errno), errno);
		return r;
	}
	free(attr);

	for (i = 0; i < dog.number; i++) {
		size_t j = dog.number - (i + 1);
		r = pthread_join(*(dog.pthread + j), NULL);
		if (r != 0) {
			syslog(LOG_ERR, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		syslog(LOG_NOTICE, "join: Thread[%lu]:0x%lx\n", 
			j, *(dog.pthread + j));
	}
	free(dog.pthread);

	closelog();
	return EXIT_SUCCESS;
}
