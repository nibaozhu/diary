#include "a.h"


const char default_task[4][3][PATH_MAX] = {
{ "9c0045ae-a9b6-492b-8a26-5646fc94d57f", "/tmp/path/to/task_where", }, 
{ "656734fc-7b91-489c-a2bf-cd2a340ba306", "/home/path/to/task_are", }, 
{ "1ed6808a-2339-46b8-b088-55d1d620eb85", "/log/path/to/task_you", }, 
{ "e536e579-2d83-4b5d-874d-d0b409dbdc0b", "/var/path/to/task_from", }, 
};

int main(int argc, char **argv) {

	/* Set sub-thread number. */
	a_t a = { 	.staff_number = 4, 
				.reception_number = 1,
				.waiter_number = 3 };
	int r;

	r = initializing(argv[0], "/tmp/test_pthread", "w+", debug, none, 0, 0, LOGGING_SIZE);
	if (r == -1)
	{
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}

	pthread_t *pthread = (pthread_t *)malloc(a.staff_number * sizeof(pthread_t));
	if (pthread == NULL) {
		return EXIT_FAILURE;
	}

	pthread_attr_t *attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
	if (attr == NULL) {
		return EXIT_FAILURE;
	}

	r = pthread_attr_init(attr);
	if (r != 0) {
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	void *(*start_routine) (void *) = NULL;
	void *arg = NULL;

	size_t i;
	for (i = 0; i < a.staff_number; i++) {

		/* personal_information will be free at sub-thread. */
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

		if (i <= a.reception_number) {
			/* First waiter is `reception'. */
			start_routine = reception;
		} else {
			start_routine = waiter;
		}

		r = pthread_create(pthread + i, (const pthread_attr_t *)attr,
						start_routine, arg);
		if (r != 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		/* XXX: `arg' maybe had been freed, and we just look it. */
		plog(notice, "create: Thread[%d]: 0x%lx, arg: %p\n", i, *(pthread + i), arg);
	}

	r = pthread_attr_destroy(attr);
	if (r != 0) {
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	free(attr);

	void *retval;

	for (i = 0; i < a.staff_number; i++) {

		r = pthread_join(*(pthread + i), &retval);
		if (r != 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		plog(notice, "join: Thread[%d]: 0x%lx\n", i, *(pthread + i));
	}

	free(pthread);

	r = uninitialized();
	if (r == -1) {
		fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
