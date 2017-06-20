#include "worker.h"


void *worker(void *arg) {

				// pid_t pid = getpid();
				pid_t tid = syscall(SYS_gettid);
				pthread_t thread = pthread_self();

				printf("[Thread 0x%lx (LWP %d)]\n", thread, tid);
				sleep(10);
				printf("[Thread 0x%lx (LWP %d)]\n", thread, tid);

				return NULL;
}
