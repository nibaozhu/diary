#include "a.h"
#include "worker.h"


int main(int argc, char **argv) {


	a_t a = { .worker_number = 4 };
	int r;


	pthread_t *pthread = (pthread_t *)malloc(a.worker_number * sizeof(pthread_t));
	if (pthread == NULL) {
		return 1;
	}

	pthread_attr_t *attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
	if (attr == NULL) {
		return 1;
	}

	r = pthread_attr_init(attr);
	if (r != 0) {
		fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	void *(*start_routine) (void *) = worker;
	void *arg = NULL;

	int i;
	for (i = 0; i < a.worker_number; i++) {
		r = pthread_create(pthread + i, (const pthread_attr_t *)attr,
  	                        start_routine, arg);
		if (r != 0) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		fprintf(stderr, "  %d Thread 0x%lx\n", 2 + i, *(pthread + i));
	}

	r = pthread_attr_destroy(attr);
	if (r != 0) {
		fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
		return r;
	}

	free(attr);


	void *retval;

	for (i = 0; i < a.worker_number; i++) {

		r = pthread_join(*(pthread + i), &retval);
		if (r != 0) {
			fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
			continue;
		}

		fprintf(stderr, "  %d Thread 0x%lx\n", 2 + i, *(pthread + i));
	}



	free(pthread);


	return 0;
}
