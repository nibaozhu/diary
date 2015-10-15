/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Written by Ni Baozhu: <nibz@qq.com>.
 */

#include <bits/local_lim.h>
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
#include <assert.h>

// -lsqlite3
#include <sqlite3.h>
sqlite3 *db;

int initializing_sqlite3(const char *argv2);
int insert_it(const char *url, const char *title);
int callback(void *NotUsed, int argc, char **argv, char **colname);

#define MAX_EVENTS (0xff)
#define IP_LENGTH (0xf)
#define BUFFER_LENGTH (0xffff)

#include "logging.h"
extern struct logging *l;

int setnonblocking(int fd);
int reads(int fd);
int writes(int fd, const char *uri, const char *host);
int do_use_fd(const char *uri, const char *host);

int quit = 0;
int fds[MAX_EVENTS] = {0};
int nconnect = 0;

/////////////////
char gURL[1024][1024];

int search_url(const char *url, const char *string, int length);
char *content = NULL;
int pos = 0;

int decoding_chunked_content(const char *content_body, int length, char *end_of_zero);
int parsing_http_protocol_response(const char *content, int length, char **chunked, int *i_content_length);

int let_us_go_this_new_url(const char *url);

int efd = 0;
struct epoll_event ev, events[MAX_EVENTS];

int usage(const char *argv0)
{
	return plog(notice, "%s www.nibaozhu.cn m1\n", argv0);
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

	char name[FILENAME_MAX];
	memset(name, 0, sizeof name);
	strcpy(name, rindex(argv[0], '/'));
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
	l->size_max = 1024*1024*10; // 1MB
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
	content = (char *)malloc(1024 * 1024 * 10);
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
		initializing_sqlite3(argv[2]);

		char sub_url[1024];
		memset(sub_url, 0, sizeof sub_url);

		snprintf(sub_url, sizeof sub_url, "http://%s", argv[1]);
		let_us_go_this_new_url(sub_url);


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

					if (quit == 1)
					{
						close(events[n].data.fd);
					}

//					do_use_fd();
				}
			}
		}
	} while (0);

	
	sqlite3_close(db);
	unset_logging();
	return retval;
}

int reads(int fd)
{
	char url[1024];
	memset(url, 0, sizeof url);

	memcpy(url, gURL[fd], sizeof url - 1);

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
		plog(debug, "pos = %d, read: retval = %u, sum = %d\n---response begin---\n%s\n---response end---\n", pos, retval, pos + retval, buffer); 

		memcpy(content + pos, buffer, retval);
		pos += retval;

//		char *strstr(const char *haystack, const char *needle);
		const char *crlf = NULL; // cr: carriage return, lf: line feed
		char *chunked = NULL;
		int i_content_length = 0;
		crlf = strstr(content, "\r\n\r\n");
		if (crlf != NULL)
		{
			parsing_http_protocol_response(content, crlf + 4 - content, &chunked, &i_content_length);
		}

		char end_of_zero = 0;
		if (chunked != NULL)
		{
			decoding_chunked_content(crlf + 4, pos - (crlf + 4 - content), &end_of_zero);
		}

		if (end_of_zero == '0' || ((i_content_length <= pos - (crlf + 4 - content)) && chunked == NULL))
		{
			search_url(url, content, pos);
			quit = 1;
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
"User-Agent: None/0.01\r\n"
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

int do_use_fd(const char *uri, const char *host)
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


int search_url(const char *url, const char *string, int length)
{
	plog(debug, "%s: %d: %s\n|||%s|||\n", __FILE__, __LINE__, __func__, string);

	regex_t preg;
	const char *regex = 
"http://\\([a-z0-9-]\\+\\.\\)\\+[a-z0-9]\\+\\(:[0-9]\\+\\)\\?\\(/[a-z0-9\\.]\\+\\)*\\(/\\)\\?" // url
; 

	regex_t preg2;
	const char *regex2 = 
"<title>[^<]\\+</title>"	// title
; 

	int cflags = REG_ICASE;

	size_t size = 0;
	int errcode = 0;
	char errbuf[1024] = {0};
	size_t errbuf_size = sizeof errbuf;

	errcode = regcomp(&preg, regex, cflags);
	if (errcode != 0)
	{
		size = regerror(errcode, &preg, errbuf, errbuf_size);
		printf("errcode = %d, %s(%u)\n", errcode, errbuf, size);
		return 1;
	}

	errcode = regcomp(&preg2, regex2, cflags);
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

	char title[1024];

	// regexec() returns zero for a successful match or REG_NOMATCH for failure.
	errcode = regexec(&preg2, string, nmatch, pmatch, eflags);
	if (errcode == 0)
	{
		memset(title, 0, sizeof title);
		memcpy(title, string + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
		plog(notice, "|||%s|||\n", title);

		insert_it(url, title);
	}

	while (errcode != REG_NOMATCH)
	{
		// regexec() returns zero for a successful match or REG_NOMATCH for failure.
		errcode = regexec(&preg, string, nmatch, pmatch, eflags);
		if (errcode == 0)
		{
			char sub_url[1024];
			memset(sub_url, 0, sizeof sub_url);
			memcpy(sub_url, string + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
			plog(notice, "|||%s|||\n", sub_url);

			do {
			if (strstr(sub_url, ".js")) { break; }
			if (strstr(sub_url, ".png")) { break; }
			if (strstr(sub_url, ".jpg")) { break; }
			if (strstr(sub_url, ".css")) { break; }
			if (strstr(sub_url, ".ico")) { break; }
			if (strstr(sub_url, "img")) { break; }
			if (strstr(sub_url, "cgi")) { break; }
			if (strstr(sub_url, "gif")) { break; }
			if (strstr(sub_url, "action.do")) { break; }
			if (strstr(sub_url, ".php")) { break; }
			if (strstr(sub_url, ".asp")) { break; }
			if (strstr(sub_url, ".jsp")) { break; }
			if (strstr(sub_url, "sta")) { break; }
			let_us_go_this_new_url(sub_url);
			} while (0);
		}

		string += pmatch[0].rm_so + 1;
	}

	regfree(&preg);
	regfree(&preg2);

	return 0;
}


int parsing_http_protocol_response(const char *content, int length, char **chunked, int *i_content_length)
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

	*chunked = strstr(string, "Transfer-Encoding: chunked\r\n");
	plog(notice, "|||chunked = %p|||\n", *chunked);
	if (chunked != NULL)
	{
		;;;;
	}

	const char *content_length = strstr(string, "Content-Length: ");
	if (content_length != NULL)
	{
		*i_content_length = atoi(content_length + strlen("Content-Length: "));
		plog(notice, "|||header_length = %d, content_length = %d, sum = %d|||\n", length, *i_content_length, length + *i_content_length);
	}

	return 0;
}


int decoding_chunked_content(const char *content_body, int length, char *end_of_zero)
{
	int chunked_size = 0;
	int i = 0;
	while (1)
	{
		if (content_body[i] >= '0' && content_body[i] <= '9')
		{
			chunked_size = chunked_size * 16 + content_body[i++] - '0';
		}
		else if (content_body[i] >= 'a' && content_body[i] <= 'f')
		{
			chunked_size = chunked_size * 16 + content_body[i++] - 'a' + 10;
		}
		else
		{
			plog(debug, "|||chunked_size = %d|||\n", chunked_size);
			if (chunked_size != 0)
			{
				i += chunked_size + 4;
				chunked_size = 0;
			}
			else if (chunked_size == 0)
			{
				*end_of_zero = '0';
				break;
			}
			else break;
		}

		if (i >= length)
		{
			break;
		}
		//*end_of_zero = content_body[i];
	}
	return 0;
}


int initializing_sqlite3(const char *argv2)
{
//	sqlite3 *db;
	char filename[PATH_MAX];
	int retval = 0;

	memset(filename, 0, sizeof filename);
	strncpy(filename, argv2, sizeof filename - 1);

	retval = sqlite3_open(filename, &db);
	if (retval != 0)
	{
		fprintf(stderr, "%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return retval;
	}

	char sql[1024];
	memset(sql, 0, sizeof sql);
	char *errmsg = NULL;
	strcpy(sql, "create table t1(url text, title text)");

	retval = sqlite3_exec(db, sql, callback, 0, &errmsg);
	if (retval != SQLITE_OK)
	{
		fprintf(stderr, "%s\n", errmsg);
		sqlite3_free(errmsg);
	}
	return 0;
}


int insert_it(const char *url, const char *title)
{
	char sql[1024];
	memset(sql, 0, sizeof sql);
	snprintf(sql, sizeof sql, "insert into t1(url, title) values('%s', '%s')", url, title);
	int retval = 0;
	char *errmsg = NULL;

	retval = sqlite3_exec(db, sql, callback, 0, &errmsg);
	if (retval != SQLITE_OK)
	{
		fprintf(stderr, "%s\n", errmsg);
		sqlite3_free(errmsg);
	}

	return 0;
}

int callback(void *NotUsed, int argc, char **argv, char **colname)
{
	int i;
	static int j = 0;
	printf("j = %d,\t", j++);
	for (i = 0; i < argc; i++)
	{
		printf("i = %d, %s = %s\t", i, colname[i], argv[i]);
	}
	printf("\n");
	return 0;
}

int let_us_go_this_new_url(const char *url)
{
	plog(emergency, "%s: %d: %s %s\n", __FILE__, __LINE__, __func__, url);
	

	do {
		///////////////////////////////////////////////
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
    	}
	
		///////////////
		const char *uri = NULL;
		char host[1024];
		memset(host, 0, sizeof host);
		uri = strstr(&url[7], "/");
		if (uri == NULL)
		{
			uri = "/";
			snprintf(host, sizeof host, "%s", &url[7]);
    	}
		else
		{
			snprintf(host, sizeof host, "%s", &url[7]);
			char *slash = strstr(host, "/");
			*slash = 0;
		}

		///////////////
	
		struct sockaddr_in addr;
		struct sockaddr_in local;
    	memset(&addr, 0, sizeof (struct sockaddr_in));
	
		addr.sin_family = AF_INET;
    	addr.sin_port = htons(80);
	
		struct hostent *ht;
		ht = gethostbyname(host);
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
	
		int retval = 0;
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
		memset(gURL[fd], 0, 1024);
		memcpy(gURL[fd], url, strlen(url));

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

		char hostname[HOST_NAME_MAX];
		size_t len = sizeof hostname;
		memset(hostname, 0, len);
		retval = gethostname(hostname, len);
		if (retval == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		}

		plog(notice, "%s|%s:%d|+>|%s|%s:%d\n", hostname, local_ip, ntohs(local.sin_port), ht->h_name, temp_addr, ntohs(addr.sin_port));

		//set non blocking
		retval = setnonblocking(fd);
		if (retval < 0)
		{
			break;
		}

		if (efd == 0) {
		efd = epoll_create(MAX_EVENTS);
		if (efd == -1)
		{
			plog(error, "%s: %d: %s %s(%d)\n", __FILE__, __LINE__, __func__, strerror(errno), errno);
			break;
		} }

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

		do_use_fd(uri, host);
	///////////////////////////////////////////////
	} while (0);

	return 0;
}



