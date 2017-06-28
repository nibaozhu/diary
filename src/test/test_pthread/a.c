#include "a.h"


#define HOTEL_NAME ("7daysinn")

const char default_task[4][3][PATH_MAX] = {
	{ "9c0045ae-a9b6-492b-8a26-5646fc94d57f", "/tmp/path/to/task_where", }, 
	{ "656734fc-7b91-489c-a2bf-cd2a340ba306", "/home/path/to/task_are", }, 
	{ "1ed6808a-2339-46b8-b088-55d1d620eb85", "/log/path/to/task_you", }, 
	{ "e536e579-2d83-4b5d-874d-d0b409dbdc0b", "/var/path/to/task_from", }, 
};

/* Set sub-thread number. */
static hotel_t hotel = { 
		.hotel_name = HOTEL_NAME,
		.staff_number = 4, 
		.reception_number = 1,
		.waiter_number = 3,
};

void handler(int signum) {

	pid_t ppid; /* the process ID of the parent of the calling process */
	pid_t pid; /* the process ID of the calling process */
	pid_t tid; /* the  caller’s thread ID (TID) */
	pthread_t ptid; /* the ID of the calling thread(This is the same value that is returned in *thread in
       the pthread_create(3) call that created this thread.) */
	
	LOGGING(notice, "{ signum: %d, ppid: %d, pid: %d, tid: %d, ptid: 0x%lx }\n", 
			signum,
			ppid = getppid(),
			pid = getpid(),
			tid = syscall(SYS_gettid),
			ptid = pthread_self()
	);

	int i;
	void *retval = NULL;
	switch (signum) {
		case SIGHUP:
			break;
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			/* main-thread: */
			if (pid == tid) exit(1);
			break;
		case SIGUSR1:
			/* sub-thread: XXX: avoid infinite recursion */
			if (pid != tid) break;
			for(i = 0; i < hotel.staff_number; i++) {
				int r = pthread_kill( *(hotel.pthread + i), signum);
				if (r != 0)
					LOGGING(error, "%s(%d)\n", strerror(r), r);
			}
			break;
		default:
			; /* do nothing */
	}
}

void set_disposition(void) {
	int i;
					/* 1) 2) 3) 15)  */
	int signum[] = { SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1};
	for (i = 0; i < sizeof (signum) / sizeof (int); i++) {
    	if (SIG_ERR == signal(signum[i], handler)) {
    		fprintf(stderr, "%s:%d: %s: signum = %d\n", __FILE__, __LINE__, __func__, signum);
			exit(1);
		}
	}
}

int main(int argc, char **argv) {

	/* logging initializing... */
	const char *name = argv[0], *path = "/tmp/test_pthread", *mode = "w+";
	enum level stream_level = debug, stdout_level = debug;
	time_t diff_max = LOGGING_INTERVAL;
	unsigned int cache_max = LOGGING_CACHE;
	unsigned long size_max = LOGGING_SIZE;

	int r = initializing(name, path, mode, 
		stream_level, stdout_level, 
		diff_max, cache_max, size_max);
	if (r == -1) return EXIT_FAILURE;

	hotel.pthread = (pthread_t *)malloc(hotel.staff_number * sizeof(pthread_t));
	if (hotel.pthread == NULL) return EXIT_FAILURE;

	pthread_attr_t *attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
	if (attr == NULL) return EXIT_FAILURE;

	r = pthread_attr_init(attr);
	if (r != 0) {
		LOGGING(critical, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	void *(*start_routine) (void *) = NULL;
	void *arg = NULL;

	size_t i;
	for (i = 0; i < hotel.staff_number; i++) {

		/* personal_information will be freed at sub-thread. */
		personal_information_t *personal_information = (personal_information_t*)malloc(sizeof(personal_information_t));
		if (personal_information == NULL) {
			return EXIT_FAILURE;
		}

		personal_information->employee_ID = i;
		personal_information->department_ID = i;

		personal_information->task_slist = (struct task_slist_s*)malloc(sizeof(struct task_slist_s));
		if(personal_information->task_slist == NULL) {
			return EXIT_FAILURE;
		}

		SLIST_INIT(personal_information->task_slist);

		int j;
		for (j = 0; j < 4; j++) {
			/* */
			task_t *task = (task_t*)malloc(sizeof(task_t));
			if (task == NULL) {
				return EXIT_FAILURE;
			}

			/* Generate a task. */
			task->ID = random();
#ifdef UUID_LEN_STR
			strncpy(task->UUID, default_task[j][0], UUID_LEN_STR);
#endif
			strncpy(task->path, default_task[j][1], PATH_MAX);

			SLIST_INSERT_HEAD(personal_information->task_slist, task, entry);
		}


		arg = personal_information;

		if (i <= hotel.reception_number) {
			/* First waiter is `reception'. */
			start_routine = reception;
		} else {
			start_routine = waiter;
		}

		r = pthread_create(hotel.pthread + i, (const pthread_attr_t *)attr,
						start_routine, arg);
		if (r != 0) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		/* XXX: `arg' maybe had been freed, and we just look it. */
		LOGGING(notice, "create: Thread[%d]: 0x%lx, arg: %p\n", i, *(hotel.pthread + i), arg);
	}

	r = pthread_attr_destroy(attr);
	if (r != 0) {
		LOGGING(critical, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	free(attr);

	/* pthread_cancel ... */
	set_disposition();

	void *retval;
	for (i = 0; i < hotel.staff_number; i++) {

		r = pthread_join(*(hotel.pthread + i), &retval);
		if (r != 0) {
			LOGGING(error, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		LOGGING(notice, "join: Thread[%d]: 0x%lx\n", i, *(hotel.pthread + i));
	}

	free(hotel.pthread);

	r = uninitialized();
	if (r == -1) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
