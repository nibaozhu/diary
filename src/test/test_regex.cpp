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

#include <iostream>
#include <string>
#include <list>

int regex_content(const char * regex, const char * string1, std::list<std::string> &urls) {
	regex_t preg;
	int cflags = REG_ICASE;

	size_t size = 0;
	int errcode = 0;
	char errbuf[1024] = {0};
	size_t errbuf_size = 1024;

	errcode = regcomp(&preg, regex, cflags);
	if (errcode != 0)
	{
		size = regerror(errcode, &preg, errbuf, errbuf_size);
		printf("errcode = %d, %s(%lu)\n", errcode, errbuf, size);
		return 1;
	}

	regmatch_t pmatch[1];
	size_t nmatch = 1;
	int eflags = REG_NOTBOL;

	while (errcode == 0)
	{
		errcode = regexec(&preg, string1, nmatch, pmatch, eflags);
		if (errcode == 0)
		{
			char its[1024];
			memset(its, 0, sizeof its);
			memcpy(its, string1 + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
			std::clog << its << std::endl;
			urls.push_back(its);
		}

		string1 += pmatch[0].rm_so + 1;
	}

	regfree(&preg);
	return 0;
}

int main(void) {
	const char *regex = "<meta\\ name=\"description\".\\+content=\"";
	const char *string1 = "<a data-href=\"http://news.baidu.com\" (? <meta name=\"description\"\r\n content=\"yes ok\" /> <title>hello WORLD</title>   <title>hello <hhhh>world</title> \r\nhref=\"http://news.baidu.com/ns?word=word&tn=news&cl=2&rn=20&ct=1&fr=wenku\" class=\"logSend\" data-lojgsend='{\"send\":[\"general\", ";
	std::list<std::string> urls;
	int ret = regex_content(regex, string1, urls);
	return ret;
}
