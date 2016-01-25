#ifndef _SPIDER_H_
#define _SPIDER_H_

#include <iostream>

#include <list>

#include <unistd.h>

#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>




#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>

#include <iostream>
#include <string>
#include <list>
#include <algorithm>

#include <csignal>

#include "logging.h"

extern bool quit;

int set_disposition();
void handler(int signum);
int regex_content(const char * regex, const char * string1, std::list<std::string> &urls);


extern std::string content;
size_t my_write(void *ptr, size_t size, size_t nmemb, void *stream);

class page {
	public:
		std::string url;
		std::string host;
		std::string ip;
		std::string title;

		std::string content;
		std::list<std::string> urls;
		bool flag;

		page() { flag = false; }
		page(std::string url, bool flag = false) {
			this->url = url;
			this->flag = flag;
		}

		bool operator< (const page &p0) {
			return this->url < p0.url;
		}

		bool operator== (const page &p0) {
			return this->url == p0.url;
		}

		int i_want_to_get_this_content() {

			if (this->flag) {
				return 0;
			}

			flag = true;

			CURL *handle;
			CURLcode errornum;
			CURLoption option = CURLOPT_URL;
			const char * parameter = this->url.c_str();

			handle = curl_easy_init( );
			if (handle == NULL) {
				return 1;
			}

			errornum = curl_easy_setopt(handle, option, parameter);
			if (errornum != CURLE_OK) {
				plog(error, "%s(%d)\n", curl_easy_strerror(errornum), errornum);
				return errornum;
			}

			option = CURLOPT_TIMEOUT;
			errornum = curl_easy_setopt(handle, option, 1);
			if (errornum != CURLE_OK) {
				plog(error, "%s(%d)\n", curl_easy_strerror(errornum), errornum);
				return errornum;
			}

			option = CURLOPT_CONNECTTIMEOUT;
			errornum = curl_easy_setopt(handle, option, 1);
			if (errornum != CURLE_OK) {
				plog(error, "%s(%d)\n", curl_easy_strerror(errornum), errornum);
				return errornum;
			}

			option = CURLOPT_ENCODING;
			errornum = curl_easy_setopt(handle, option, "deflate,gzip");
			if (errornum != CURLE_OK) {
				plog(error, "%s(%d)\n", curl_easy_strerror(errornum), errornum);
				return errornum;
			}

			option = CURLOPT_WRITEFUNCTION;
			errornum = curl_easy_setopt(handle, option, &my_write);
			if (errornum != CURLE_OK) {
				plog(error, "%s(%d)\n", curl_easy_strerror(errornum), errornum);
				return errornum;
			}

			plog(notice, "GET %s\n", this->url.c_str());
			errornum = curl_easy_perform(handle);
			if (errornum != CURLE_OK) {
				plog(error, "%s(%d)\n", curl_easy_strerror(errornum), errornum);
				return errornum;
			}

			curl_easy_cleanup(handle);

			this->content = ::content;

			std::string regex = "http://\\([a-z0-9:@-]\\+\\.\\)\\+[a-z0-9:@-]\\+\\(:[0-9]\\+\\)\\?\\(/[a-z0-9\\.\\?=&@-]\\+\\)*\\(/\\)\\?";
			regex_content(regex.c_str(), this->content.c_str(), this->urls);
			plog(notice, "this->urls.size() = %d\n", this->urls.size());

			::content.clear();
			return 0;
		}

};


class spider {
	public:
		page root;
		std::list<page> urls;

		spider(std::string root_url) {
			plog(debug, "Construct: %s\n", __func__);
			this->root.url = root_url;
			plog(debug, "this->root.url = %s\n", this->root.url.c_str());

			urls.push_back(this->root);
		}

		int run() {

			do {
				this->doing();
			} while (!quit);

			std::list<page>::iterator i = this->urls.begin();
			for (i = this->urls.begin(); i != this->urls.end(); i++) {
				page p = *i;
				plog(notice, "|%s|\n", p.url.c_str());
			}

			plog(notice, "this->urls.size() = %d\n", this->urls.size());
			int ret = 0;
			return ret;
		}

		int doing() {
			plog(debug, ":..: %s\n", __func__);

			std::list<page>::iterator i = this->urls.begin();

			plog(info, "before this->urls.size() = %d\n", this->urls.size());

			while (i != this->urls.end() && !quit) {
				page p = *i;
				this->get_page(p);

				p.urls.sort();
				p.urls.unique();

				std::list<std::string>::iterator iu = p.urls.begin();
				while (iu != p.urls.end()) {
					std::string s1 = *iu;
					bool flag = false;
					if (s1 == p.url) {
						flag = true;
					}

					page pi(s1, flag);
					this->urls.push_front(pi);
					iu++;
				}
				i++;
			}

			std::reverse(this->urls.begin(), this->urls.end());
			this->urls.sort();
			this->urls.unique();

			for (i = this->urls.begin(); i != this->urls.end(); i++) {
				page p = *i;
				plog(info, "p.url = %s\n", p.url.c_str());
			}

			plog(info, "after this->urls.size() = %d\n", this->urls.size());
			int ret = 0;
			return ret;
		}

		int get_page(page &p) {

			p.i_want_to_get_this_content();

			int ret = p.content.size();
			return ret;
		}

		~spider() {
			plog(debug, "Deconstruct: %s\n", __func__);
		}
};

#endif
