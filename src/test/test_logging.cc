#include "logging.h"
extern struct logging *l;

#include <strings.h>
#include <signal.h>

unsigned int quit = 0;

int do_something(enum elevel x, int timeout)
{
	plog(x, "This is a logging message. timeout = %d\n", timeout);
	usleep(timeout);
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
	enum elevel x = none;
	int to = 0;
	while (!quit)
	{

//	emergency,		/* application is unusable */
//	alert,			/* action must be taken immediately */
//	critical,		/* critical conditions */
//	error,			/* error conditions */
//	warning,		/* warning conditions */
//	notice,			/* normal but significant condition */
//	info,			/* informational */
//	debug,			/* debug-level messages */
		to = rand() % 1000000;
		x = emergency; do_something(x, to);
		to = rand() % 1000000;
		x = alert; do_something(x, to);
		to = rand() % 1000000;
		x = critical; do_something(x, to);
		to = rand() % 1000000;
		x = error; do_something(x, to);
		to = rand() % 1000000;
		x = warning; do_something(x, to);
		to = rand() % 1000000;
		x = notice; do_something(x, to);
		to = rand() % 1000000;
		x = info; do_something(x, to);
		to = rand() % 1000000;
		x = debug; do_something(x, to);
	}

	retval = uninitialized();
	free(l);
	return retval;
}
