#include "a.h"


#define HOTEL_NAME ("7daysinn")

const char default_task[4][3][PATH_MAX] = {
	{ "9c0045ae-a9b6-492b-8a26-5646fc94d57f", "/tmp/path/to/task_where", }, 
	{ "656734fc-7b91-489c-a2bf-cd2a340ba306", "/home/path/to/task_are", }, 
	{ "1ed6808a-2339-46b8-b088-55d1d620eb85", "/log/path/to/task_you", }, 
	{ "e536e579-2d83-4b5d-874d-d0b409dbdc0b", "/var/path/to/task_from", }, 
};

/* Set sub-thread number. */
hotel_t hotel = { 
		.hotel_name = HOTEL_NAME,
		.staff_number = 4, 
		.reception_number = 1,
		.waiter_number = 3,
		.bankruptcy = false,
};

void handler(int signum) {
	pid_t ppid, pid, tid;
	pthread_t ptid;

	syslog(LOG_NOTICE, "{ signum:%d, ppid:%d, pid:%d, tid:%d, ptid:0x%lx }\n", 
			signum,
			ppid = getppid(), pid = getpid(), tid = syscall(SYS_gettid),
			ptid = pthread_self()
	);

	switch (signum) {
		case SIGHUP :
			break;
		case SIGINT :
		case SIGQUIT:
		case SIGTERM:
		case SIGUSR1:
		case SIGUSR2:
			if (pid == tid) hotel.bankruptcy = true; // FIXME: rwlock
			break;
		default: ; /* do nothing */
	}
}

void set_disposition(void) {
	size_t i, signum_arr[] = {SIGHUP, SIGINT, SIGQUIT, SIGTERM, 
					SIGUSR1, SIGUSR2};
	for (i = 0; i < sizeof (signum_arr) / sizeof (size_t); i++) {
		if (SIG_ERR == signal(signum_arr[i], handler)) {
			syslog(LOG_CRIT, "signum_arr[%d]:%lu\n", i, signum_arr[i]);
			exit(signum_arr[i]);
		}
	}
}

int main(int argc, char **argv) {
	const char *ident = basename(argv[0]);
	int option = LOG_CONS | LOG_PID;
	int facility = LOG_USER;
	openlog(ident, option, facility);

	set_disposition();
	hotel.pthread = 
		(pthread_t *)malloc(hotel.staff_number * sizeof(pthread_t));
	if (hotel.pthread == NULL) return EXIT_FAILURE;

	pthread_attr_t *attr = 
		(pthread_attr_t *)malloc(sizeof(pthread_attr_t));
	if (attr == NULL) return EXIT_FAILURE;

	int r = pthread_attr_init(attr);
	if (r != 0) {
		syslog(LOG_CRIT, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	size_t i;
	for (i = 0; i < hotel.staff_number; i++) {

		/* personal_information_t will be freed at sub-thread. */
		personal_information_t *personal_information = 
			(personal_information_t*)malloc(sizeof(personal_information_t));
		if (personal_information == NULL) {
			return EXIT_FAILURE;
		}

		personal_information->employee_ID = i;
		personal_information->department_ID = i;

		personal_information->task_slist = 
			(struct task_slist_s*)malloc(sizeof(struct task_slist_s));
		if(personal_information->task_slist == NULL) return EXIT_FAILURE;
		SLIST_INIT(personal_information->task_slist);

		int j;
		for (j = 0; j < 4; j++) {
			/* */
			task_t *task = (task_t*)malloc(sizeof(task_t));
			if (task == NULL) return EXIT_FAILURE;

			/* Generate a task. */
			task->ID = random();
#ifdef UUID_LEN_STR
			strncpy(task->UUID, default_task[j][0], UUID_LEN_STR);
#endif
			strncpy(task->path, default_task[j][1], PATH_MAX);

			SLIST_INSERT_HEAD(personal_information->task_slist, task, entry);
		}

		void *arg = personal_information;
		void *(*start_routine) (void *) = NULL;

		if (i <= hotel.reception_number) {
			start_routine = reception; /* First waiter is `reception'. */
		} else {
			start_routine = waiter;
		}

		r = pthread_create(hotel.pthread + i, (const pthread_attr_t *)attr,
						start_routine, arg);
		if (r != 0) {
			syslog(LOG_ERR, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		/* XXX: `arg' maybe had been freed, and we just look it. */
		syslog(LOG_NOTICE, "create: Thread[%lu]:0x%lx, arg:%p\n", 
			i, *(hotel.pthread + i), arg);
	}

	r = pthread_attr_destroy(attr);
	if (r != 0) {
		syslog(LOG_CRIT, "%s(%d)\n", strerror(errno), errno);
		return r;
	}
	free(attr);

	for (i = 0; i < hotel.staff_number; i++) {
		size_t j = hotel.staff_number - (i + 1);
		r = pthread_join(*(hotel.pthread + j), NULL);
		if (r != 0) {
			syslog(LOG_ERR, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		syslog(LOG_NOTICE, "join: Thread[%lu]:0x%lx\n", 
			j, *(hotel.pthread + j));
	}
	free(hotel.pthread);

	closelog();
	return EXIT_SUCCESS;
}
