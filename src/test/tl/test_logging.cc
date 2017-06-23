#include "logging.h"

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
	fprintf(stderr, "%s %s:%d: %s: signum = %d\n", level[notice], __FILE__, __LINE__, __func__, signum);
	if (signum == SIGINT || signum == SIGTERM )
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
	int retval = initializing(argv[0], "/tmp/test_logging", "w+", debug, debug, LOGGING_INTERVAL, LOGGING_CACHE, LOGGING_SIZE);
	if (retval == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	retval = set_disposition();
	if (retval == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	while (!quit)
	{
		int to = rand() % 1000000;
		enum elevel x = (enum elevel)(rand() % debug);
		do_something(x, to);
	}

	retval = uninitialized();
	return retval;
}
