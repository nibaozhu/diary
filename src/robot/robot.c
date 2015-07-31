/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Written by Ni Baozhu: <nibz@qq.com>.
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <regex.h>
#include <netdb.h>
extern int h_errno;

#define MAX_EVENTS (0xff)
#define IP_LENGTH (0xf)
#define BUFFER_LENGTH (0xffff)

#include "logging.h"
struct logging *l;

int setnonblocking(int fd);
int reads(int fd);
int writes(int fd, const char *uri, const char *host);
int do_use_fd();
int quit = 0;
int fds[MAX_EVENTS] = {0};
int nconnect = 0;

/////////////////
char host[1024];
char uri[1024] = "/";

int search_url(const char *string, int length);
char *content = NULL;
int pos = 0;

int parsing_http_protocol_response(const char *content, int length);


int usage(const char *argv0)
{
	return printf("%s www.nibaozhu.cn 80 \"/project/index.html\"\n", argv0);
}

void handler(int signum)
{
	pflush();
	plog(notice, "%s: %d: %s signum = %d\n", __FILE__, __LINE__, __func__, signum);
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

int set_logging(int argc, const char **argv)
{
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
	// YEAR			MONTH			DAY				HOUR			MINUTE			SECOND
	t2.tm_year = 0; t2.tm_mon = 0; t2.tm_mday = 0; t2.tm_hour = 0; t2.tm_min = 0; t2.tm_sec = 0;

	l->diff = t2.tm_sec + t2.tm_min * 60 + t2.tm_hour * 60 * 60 + t2.tm_mday * 60 * 60 * 24 + t2.tm_mon * 60 * 60 * 24 * 30 + t2.tm_year * 60 * 60 * 24 * 30 * 365;
	l->pid = getpid();
	l->cache_max = 0;
	l->size_max = 1024*1024*1; // 1MB
	strncpy(l->path, "../../log", sizeof l->path - 1);
	strncpy(l->mode, "w+", sizeof l->mode - 1);
	l->stream_level = debug;
	l->stdout_level = debug;

	int retval = initializing();
	return retval;
}

int unset_logging()
{
	int retval = uninitialized();
	free(l);
	return retval;
}


int main(int argc, char **argv)
{
	content = malloc(1024 * 1024);
	memset(content, 0, sizeof content);

	int retval = set_logging(argc, (const char**)argv);
	if (retval == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	retval = set_disposition();
	if (retval == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	// print the program, pid, release
	plog(info, "PROGRAM: %s, PID: %u, RELEASE: %s %s\n", l->name, l->pid, __DATE__, __TIME__);

	do {
		if (argc < 3)
		{
			usage(argv[0]);
			break;
		}

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		///////////////
		memset(host, 0, sizeof host);
		memcpy(host, argv[1], sizeof host - 1);

		///////////////
		memset(uri, 0, sizeof uri);
		memcpy(uri, argv[3], sizeof uri - 1);

		struct sockaddr_in addr;
		struct sockaddr_in local;
		memset(&addr, 0, sizeof (struct sockaddr_in));

		addr.sin_family = AF_INET;
		addr.sin_port = htons(atoi(argv[2]));

		struct hostent *ht;
		ht = gethostbyname(argv[1]);
		if (ht == NULL)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, hstrerror(h_errno), h_errno);
			break;
		}

		char temp_addr[1024] = {0};
		socklen_t slt = sizeof temp_addr;
		char *ri;

		// On success, inet_ntop() returns a non-null pointer to dst.  NULL is returned if there was an error, with errno set to indicate the error.
		inet_ntop(AF_INET, (void *)*(ht->h_addr_list), temp_addr, slt);
		if (temp_addr == NULL)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		retval = inet_pton(AF_INET, temp_addr, (struct sockaddr *) &addr.sin_addr.s_addr);
		if (retval == 0)
		{
			plog(error, "%s: %d: %s %s\n", __FILE__, __LINE__, __func__, 
"0 is returned if src does not contain a character string representing a valid network address in the specified address family."
			);
			break;
		}
		else if (retval == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		socklen_t addrlen = sizeof (struct sockaddr_in);
		retval = connect(fd, (struct sockaddr *) &addr, addrlen);
		if (retval == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		fds[nconnect++] = fd;

		socklen_t locallen = sizeof (struct sockaddr_in);
		retval = getsockname(fd, (struct sockaddr *) &local, &locallen);
		if (retval == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		char local_ip[IP_LENGTH];
		memset(local_ip, 0, sizeof local_ip);
		if (inet_ntop(AF_INET, (void*) &local.sin_addr.s_addr, local_ip, locallen) == NULL)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		plog(notice, "localhost %s:%d | connect to %s|%s|:%d\n", local_ip, ntohs(local.sin_port), argv[1], temp_addr, ntohs(addr.sin_port));

		//set non blocking
		retval = setnonblocking(fd);
		if (retval < 0)
		{
			break;
		}

		int efd = epoll_create(MAX_EVENTS);
		if (efd == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		struct epoll_event ev, events[MAX_EVENTS];
		ev.events = EPOLLIN | EPOLLET; //epoll edge triggered
		//      ev.events = EPOLLIN | EPOLLOUT | EPOLLET; // epoll edge triggered
		//      ev.events = EPOLLIN | EPOLLOUT          ; // epoll level triggered (default)
		ev.data.fd = fd;
		retval = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
		if (retval == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		do_use_fd();

		int n = 0;
		while (1)
		{
			if (quit == 1)
			{
				break;
			}

			int nfds = epoll_wait(efd, events, MAX_EVENTS, 1000);
			if (nfds == -1)
			{
				plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
				break;
			}

			for (n = 0; n < nfds; n++)
			{
				if (events[n].events & EPOLLHUP)
				{
					plog(error, "%s: %d: %s "
"Hang up happened on the associated file descriptor."
					"(events[%d].events = 0x%03x)\n", __FILE__, __LINE__, __func__, n, events[n].events);

					retval = epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (retval == -1)
					{
						plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
						break;
					}
					close(events[n].data.fd);
					quit = 1;
					break;
				}

				if (events[n].events & EPOLLERR)
				{
					plog(error, "%s: %d: %s "
"Error condition happened on the associated file descriptor.  epoll_wait(2) will always wait for this event; it is not necessary to set it in events."
					"(events[%d].events = 0x%03x)\n", __FILE__, __LINE__, __func__, n, events[n].events);

					retval = epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &events[n]);
					if (retval == -1)
					{
						plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
						break;
					}
					close(events[n].data.fd);
					quit = 1;
					break;
				}

				if (events[n].events & EPOLLIN)
				{
					plog(info, "%s: %d: %s "
"The associated file is available for read(2) operations."
					"(events[%d].events = 0x%03x)\n", __FILE__, __LINE__, __func__, n, events[n].events);

					retval = reads(events[n].data.fd);
					if (retval < 0)
					{
						break;
					}

//					do_use_fd();
				}
			}
		}
	} while (0);

	unset_logging();

	return retval;
}

int reads(int fd)
{
	int retval = 0;
	do {
		char buffer[BUFFER_LENGTH];
		memset(buffer, 0, sizeof buffer);
		retval = read(fd, buffer, BUFFER_LENGTH);
		if (retval < 0)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}
		plog(debug, "\n---response begin---\n%s\n---response end---\n", buffer); 
		if (retval == 0)
		{
			search_url(content, pos);
		}

		memcpy(content + pos, buffer, retval);
		pos += retval;

//		char *strstr(const char *haystack, const char *needle);
		const char *crlf = NULL; // cr = carriage return, lf: line feed
		crlf = strstr(content, "\r\n\r\n");
		if (crlf != NULL)
		{
			parsing_http_protocol_response(content, crlf - content);
		}

	} while (0);
	return retval;
}

int writes(int fd, const char *uri, const char *host)
{
	int retval = 0;
	do {
		char buffer[BUFFER_LENGTH];
		memset(buffer, 0, sizeof buffer);

		sprintf(buffer,
/* -http protocol request message header START- */
"GET %s HTTP/1.1\r\n"
"User-Agent: Wget/1.12 (linux-gnu)\r\n"
"Accept: */*\r\n"
"Host: %s\r\n"
"Connection: Keep-Alive\r\n"
"\r\n"
/* -http protocol request message header ENDED- */
		, uri, host);

		size_t length = strlen(buffer);
		retval = write(fd, buffer, length);
		if (retval < 0)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}
		plog(notice, "\n---request begin---\n"
"%s\n"
		"---request end---\n", buffer);
	} while (0);
	return retval;
}

int setnonblocking(int fd)
{
	int retval = 0;
	do {
		int flags = fcntl(fd, F_GETFL);	
		if (flags == -1)
		{
			retval = flags;
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}
		flags |= O_NONBLOCK; //non block
		retval = fcntl(fd, F_SETFL, flags);
		if (retval == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}
	} while (0);
	return retval;
}

int do_use_fd()
{
	int retval = 0;
	int n = 0;
	do {
		for (n = 0; n < nconnect; n++)
		{
			retval = writes(fds[n], uri, host);
			if (retval < 0)
			{
				break;
			}
		}
	} while (0);
	return retval;
}


int search_url(const char *string, int length)
{
	plog(info, "|||%s|||\n", string);  

	regex_t preg;
	const char *regex = "http://\\([a-z0-9]\\+\\.\\)\\+[a-z0-9]\\+\\(:[0-9]\\+\\)\\?\\(/[a-z0-9\\.\\?=&]\\+\\)*\\(/\\)\\?";
	int cflags = REG_ICASE;

	size_t size = 0;
	int errcode = 0;
	char errbuf[1024] = {0};
	size_t errbuf_size = 1024;

	errcode = regcomp(&preg, regex, cflags);
	if (errcode != 0)
	{
		size = regerror(errcode, &preg, errbuf, errbuf_size);
		printf("errcode = %d, %s(%u)\n", errcode, errbuf, size);
		return 1;
	}

//	const char *string = "how are you? yes. http://www9.sina.com.cn:80/news/index.html hi, very good.\n";
	regmatch_t pmatch[1];
	size_t nmatch = 1;
	int eflags = REG_NOTBOL;

	while (errcode == 0)
	{
		// regexec() returns zero for a successful match or REG_NOMATCH for failure.
		errcode = regexec(&preg, string, nmatch, pmatch, eflags);
		if (errcode == 0)
		{
			char its[1024];
			memset(its, 0, sizeof its);
			memcpy(its, string + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
			plog(notice, "|||%s|||\n", its);
		}

		string += pmatch[0].rm_so + 1;
	}

	regfree(&preg);

	return 0;
}


int parsing_http_protocol_response(const char *content, int length)
{
//	char *strstr(const char *haystack, const char *needle);
	char string[1024];
	memset(string, 0, sizeof string);
	if (length >= 1024)
	{
		return 1;
	}

	memcpy(string, content, length);

//	HTTP/1.1 400
	const char *return_code = strstr(string, "HTTP/1.1 200");
	if (return_code == NULL)
	{
		return 1;
	}


	const char *chunked = strstr(string, "Transfer-Encoding: chunked\r\n");
	plog(notice, "|||chunked = %p|||\n", chunked);

	const char *content_length = strstr(string, "Content-Length: ");
	if (content_length != NULL)
	{
		plog(notice, "|||content_length = %d|||\n", atoi(content_length + strlen("Content-Length: ")));
	}

	return 0;
}

