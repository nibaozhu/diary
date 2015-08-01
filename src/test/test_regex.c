/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Written by Ni Baozhu: <nibz@qq.com>.
 */

#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>


int main(int argc, char **argv)
{
//	int regcomp(regex_t *preg, const char *regex, int cflags);
	regex_t preg;
	//const char *regex = "http://\\([a-z0-9]\\+\\.\\)\\+[a-z0-9]\\+\\(:[0-9]\\+\\)\\?\\(/[a-z0-9\\.\\?=&]\\+\\)*\\(/\\)\\?";

	//(?<=<title>)[^<]+(?=</title>)
	//const char *regex = "\\(\\?<=<title>\\)[^<]\\+\\(\\?=</title>\\)";
	// const char *regex = "<title>[^<]\\+</title>";
	// <meta\\ name=\"description\"\\.\\+content=\"[^<]\\+/>
	const char *regex = "<meta\\ name=\"description\".\\+content=\"";
	int cflags = REG_ICASE;

	size_t size = 0;
	int errcode = 0;
	char errbuf[1024] = {0};
	size_t errbuf_size = 1024;

	errcode = regcomp(&preg, regex, cflags);
	if (errcode != 0)
	{
//	size_t regerror(int errcode, const regex_t *preg, char *errbuf,
//                 size_t errbuf_size);
		size = regerror(errcode, &preg, errbuf, errbuf_size);
		printf("errcode = %d, %s(%u)\n", errcode, errbuf, size);
		return 1;
	}

	//const char *string = "<a data-href=\"http://news.baidu.com\" \r\nhref=\"http://news.baidu.com/ns?word=word&tn=news&cl=2&rn=20&ct=1&fr=wenku\" class=\"logSend\" data-lojgsend='{\"send\":[\"general\", ";

	const char *string = "<a data-href=\"http://news.baidu.com\" (? <meta name=\"description\"\r\n content=\"yes ok\" /> <title>hello WORLD</title>   <title>hello <hhhh>world</title> \r\nhref=\"http://news.baidu.com/ns?word=word&tn=news&cl=2&rn=20&ct=1&fr=wenku\" class=\"logSend\" data-lojgsend='{\"send\":[\"general\", ";
	regmatch_t pmatch[1];
	size_t nmatch = 1;
	int eflags = REG_NOTBOL;

	while (errcode == 0)
	{
//	int regexec(const regex_t *preg, const char *string, size_t nmatch,
//                   regmatch_t pmatch[], int eflags);
		// regexec() returns zero for a successful match or REG_NOMATCH for failure.
		errcode = regexec(&preg, string, nmatch, pmatch, eflags);
		if (errcode == 0)
		{
			char its[1024];
			memset(its, 0, sizeof its);
			memcpy(its, string + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
			puts(its);
		}

		string += pmatch[0].rm_so + 1;
	}

//	void regfree(regex_t *preg);
	regfree(&preg);
	return 0;
}

