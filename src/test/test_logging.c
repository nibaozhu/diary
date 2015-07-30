#include "logging.h"
struct logging *l;

#include <strings.h>
#include <signal.h>

unsigned int quit = 0;

int do_something(enum elevel x)
{
	plog(x, "This is a (logging level = %d) message. %s = \"%p\"\n", x, __func__, __func__);
	return 0;
}

void handler(int signum)
{
	pflush();
	fprintf(stderr, "%s %s:%d: %s: signum = %d\n", level[notice], __FILE__, __LINE__, __func__, signum);

	if (signum == SIGINT || signum == SIGTERM || signum == SIGSEGV)
	{
		quit = 1;
	}
	return ;
}

int set_disposition()
{
	int retval = 0;
	do {
		if (signal(SIGINT, handler) == SIG_ERR)
		{
			plog(error, "%s(%d)\n", strerror(errno), errno);
			retval = EXIT_FAILURE;
			break;
		}
		if (signal(SIGSEGV, handler) == SIG_ERR)
		{
			plog(error, "%s(%d)\n", strerror(errno), errno);
			retval = EXIT_FAILURE;
			break;
		}
		if (signal(SIGTERM, handler) == SIG_ERR)
		{
			plog(error, "%s(%d)\n", strerror(errno), errno);
			retval = EXIT_FAILURE;
			break;
		}
	} while (0);
	return retval;
}

int main(int argc, char **argv)
{
	int retval = 0;
	retval = set_disposition();
	if (retval == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	l = (struct logging*) malloc(sizeof (struct logging));
	memset(l, 0, sizeof *l);

	char *name;
	name = rindex(argv[0], '/');
	if (name == NULL)
	{
		strncpy(l->name, argv[0], sizeof l->name - 1);
	}
	else
	{
		strncpy(l->name, name + 1, sizeof l->name - 1);
	}

	struct timeval t0;
	// gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)).
	gettimeofday(&t0, NULL);

	// When interpreted as an absolute time value, it represents the number of seconds elapsed since 00:00:00
	//	on January 1, 1970, Coordinated Universal Time (UTC).
	localtime_r(&t0.tv_sec, &l->t0);

	struct tm t2;
	t2.tm_year = 0;
	t2.tm_mon = 0;
	t2.tm_mday = 0;
	t2.tm_hour = 0;
	t2.tm_min = 0;
	t2.tm_sec = 5;

	l->diff = t2.tm_sec + t2.tm_min * 60 + t2.tm_hour * 60 * 60 + t2.tm_mday * 60 * 60 * 24 + t2.tm_mon * 60 * 60 * 24 * 30 + t2.tm_year * 60 * 60 * 24 * 30 * 365;
	l->pid = getpid();
	l->cache_max = 23;
	l->size_max = 1024*1024*1; // 1MB
	strncpy(l->path, "../../log", sizeof l->path - 1);
	strncpy(l->mode, "w+", sizeof l->mode - 1);
	l->stream_level = debug;
	l->stdout_level = debug;

	retval = initializing();
	if (retval == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	// print the program name , pid, release
	plog(info, "PROGRAM: %s, PID: %u, RELEASE: %s %s\n", l->name, l->pid, __DATE__, __TIME__);

	srand(l->pid);
	int i = 0;
	while (!quit)
	{
		int x = (i++) % (debug + 1);
		do_something(x);
		usleep(rand() % 1000000);
	}

	retval = uninitialized();
	free(l);
	return retval;
}
