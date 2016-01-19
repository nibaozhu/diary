

#include "spider.h"


std::string content;

size_t my_write(void *ptr, size_t size, size_t nmemb, void *stream) {
	// fwrite(ptr, size, nmemb, stream);
	// printf("ptr = %p,\tsize = %lx,\tnmemb = %lx,\tstream = %p\n", ptr, size, nmemb, stream);

	content.append((char *)ptr, size * nmemb);
	return size * nmemb;
}

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
		do {
			errcode = regexec(&preg, string1, nmatch, pmatch, eflags);
			if (errcode == 0)
			{
				char sub_url[1024];
				memset(sub_url, 0, sizeof sub_url);
				memcpy(sub_url, string1 + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
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
				if (strstr(sub_url, ".aspx")) { break; }
				if (strstr(sub_url, ".jsp")) { break; }
				if (strstr(sub_url, "sta")) { break; }
				if (strstr(sub_url, "ccnt")) { break; }
				if (strstr(sub_url, ".svg")) { break; }
				if (strstr(sub_url, "?")) { break; }
				if (strstr(sub_url, ".org")) { break; }

				char *ix = index(sub_url + 7, '/');
				if (ix != NULL) {
					*ix = NULL;
				}

				// std::clog << sub_url << std::endl;
				urls.push_back(sub_url);
			}
		} while (false);

		string1 += pmatch[0].rm_so + 1;
	}

	urls.unique();
	regfree(&preg);
	return 0;
}

