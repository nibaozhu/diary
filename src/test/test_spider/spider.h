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

		page() { ; }
		page(std::string url) {
			this->url = url;
		}

		bool operator< (const page &p0) {
			return this->url < p0.url;
		}

		bool operator== (const page &p0) {
			return this->url == p0.url;
		}

		int i_want_to_get_this_content() {

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
				std::clog << curl_easy_strerror(errornum) << std::endl;
				return errornum;
			}

			option = CURLOPT_TIMEOUT;
			errornum = curl_easy_setopt(handle, option, 1);
			if (errornum != CURLE_OK) {
				std::clog << curl_easy_strerror(errornum) << std::endl;
				return errornum;
			}

			option = CURLOPT_CONNECTTIMEOUT;
			errornum = curl_easy_setopt(handle, option, 1);
			if (errornum != CURLE_OK) {
				std::clog << curl_easy_strerror(errornum) << std::endl;
				return errornum;
			}

			option = CURLOPT_ENCODING;
			errornum = curl_easy_setopt(handle, option, "deflate,gzip");
			if (errornum != CURLE_OK) {
				std::clog << curl_easy_strerror(errornum) << std::endl;
				return errornum;
			}

			option = CURLOPT_WRITEFUNCTION;
			errornum = curl_easy_setopt(handle, option, &my_write);
			if (errornum != CURLE_OK) {
				std::clog << curl_easy_strerror(errornum) << std::endl;
				return errornum;
			}

			std::clog << "GET " << this->url.c_str() << std::endl;
			errornum = curl_easy_perform(handle);
			if (errornum != CURLE_OK) {
				std::clog << this->url.c_str() << curl_easy_strerror(errornum) << std::endl;
				return errornum;
			}

			curl_easy_cleanup(handle);

			this->content = ::content;

			std::string regex = "http://\\([a-z0-9:@-]\\+\\.\\)\\+[a-z0-9:@-]\\+\\(:[0-9]\\+\\)\\?\\(/[a-z0-9\\.\\?=&@-]\\+\\)*\\(/\\)\\?";
			regex_content(regex.c_str(), this->content.c_str(), this->urls);
			std::clog << "URLS " << this->urls.size() << std::endl;

			::content = "";
			return 0;
		}

};


class spider {
	public:
		page root;
		std::list<page> urls;

		spider(std::string root_url) {
			std::clog << "Construct: " << __func__<< std::endl;
			this->root.url = root_url;
			std::clog << "this->root.url = " << this->root.url << std::endl;

			urls.push_back(this->root);
		}

		int run() {

			do {
				this->doing();
			} while (true);

			int ret = 0;
			return ret;
		}

		int doing() {
			std::clog << ": " << __func__<< std::endl;

			std::list<page>::iterator i = this->urls.begin();

			std::clog << "before this->urls.size = " << this->urls.size() << std::endl;

			while (i != this->urls.end()) {
				page p = *i;
				this->get_page(p);

				p.urls.sort();
				p.urls.unique();

				std::list<std::string>::iterator iu = p.urls.begin();
				while (iu != p.urls.end()) {
					std::string s1 = *iu;
					page pi(s1);
					this->urls.push_front(pi);
					iu++;
				}
				i++;
			}

			this->urls.sort();
			this->urls.unique();

			for (i = this->urls.begin(); i != this->urls.end(); i++) {
				page p = *i;
				std::clog << p.url << std::endl;
			}

			std::clog << "after  this->urls.size = " << this->urls.size() << std::endl;
			int ret = 0;
			return ret;
		}

		int get_page(page &p) {

			p.i_want_to_get_this_content();

			int ret = p.content.size();
			return ret;
		}

		~spider() {
			std::clog << "Deconstruct: " << __func__<< std::endl;
		}
};

#endif
