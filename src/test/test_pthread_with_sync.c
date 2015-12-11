#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


pthread_mutex_t mutex; // = PTHREAD_MUTEX_INITIALIZER;
int count = 0;

void *start_routine(void *arg) {
	pid_t pid = getpid();
	pthread_t ptid = pthread_self();
	printf("process_id = %d (0x%lx), thread_id = %lu (0x%lx), [%s] %d\n", pid, pid, ptid, ptid, (char*)arg, count++);
	pthread_mutex_unlock(&mutex);
	return arg;
}

#define __MAX (100)

int main() {
	int ret = 0;

	/* int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg); */
	pthread_t *thread = (pthread_t*)malloc(sizeof (pthread_t) * (__MAX));
	const pthread_attr_t *attr = NULL;
	void *arg;

	arg = malloc(5);
	memset(arg, 0, sizeof arg);

	/*
	   int pthread_mutex_lock(pthread_mutex_t *mutex);
	   int pthread_mutex_trylock(pthread_mutex_t *mutex);
	   int pthread_mutex_unlock(pthread_mutex_t *mutex);
	   */

	/*
	   int pthread_mutex_destroy(pthread_mutex_t *mutex);
	   int pthread_mutex_init(pthread_mutex_t *restrict mutex,
	   const pthread_mutexattr_t *restrict attr);
	   pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	   */

	ret = pthread_mutex_init(&mutex, NULL);
	if (ret != 0) {
		printf("%s(%d)\n", strerror(ret), ret);
		return ret;
	}

	pid_t pid = getpid();
	pthread_t ptid = pthread_self();
	printf("process_id = %d (0x%lx), thread_id = %lu (0x%lx)\n", pid, pid, ptid, ptid);

	int i;
	for (i = 0; i < __MAX; i++) {


	pthread_mutex_lock(&mutex);
		sprintf(arg, "Y%d", i);
		ret = pthread_create(thread + i, attr, start_routine, arg);
		if (ret != 0) {
			printf("%s(%d)\n", strerror(ret), ret);
			continue;
		}


		if (i == -1) {
			ret = pthread_detach(*(thread + i));
			if (ret != 0) {
				printf("%s(%d)\n", strerror(ret), ret);
				continue;
			}
		}
	}

	/* int pthread_join(pthread_t thread, void **retval); */
	for (i = 0; i < __MAX; i++) {
		ret = pthread_join(*(thread + i), &arg);
		if (ret != 0) {
			printf("%s(%d)\n", strerror(ret), ret);
			continue;
		}
	}

	free(arg);
	free(thread);
	return ret;
}
