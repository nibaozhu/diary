#include "a.h"


int main(int argc, char **argv) {

	// NOTE: Just use 4 sub-thread.
	a_t a = { .waiter_number = 4 };
	int r;

	r = initializing(argv[0], "./", "w+", debug, debug, 0, 1, 1024 * 1024);
	if (r == -1)
	{
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}

	pthread_t *pthread = (pthread_t *)malloc(a.waiter_number * sizeof(pthread_t));
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

	int i;
	for (i = 0; i < a.waiter_number; i++) {

		if (i == 0) {
			// NOTE: First waiter is `reception'.
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

		// NOTE: Normally main-thread stands for 1, and sub-thread starts with 2 in `gdb shell'.
		plog(notice, "  %d Thread 0x%lx\n", 2 + i, *(pthread + i));
	}

	r = pthread_attr_destroy(attr);
	if (r != 0) {
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	free(attr);

	void *retval;

	for (i = 0; i < a.waiter_number; i++) {

		r = pthread_join(*(pthread + i), &retval);
		if (r != 0) {
			plog(error, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		plog(notice, "  %d Thread 0x%lx\n", 2 + i, *(pthread + i));
	}



	free(pthread);

	r = uninitialized();
	if (r == -1) {
		plog(critical, "%s(%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
