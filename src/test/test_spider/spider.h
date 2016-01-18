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

extern std::string content;
size_t my_write(void *ptr, size_t size, size_t nmemb, void *stream);

class page {
	public:
		std::string url;
		std::string host;
		std::string ip;
		std::string title;

		std::string content;

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
				puts(curl_easy_strerror(errornum));
				return errornum;
			}

			option = CURLOPT_ENCODING;
			errornum = curl_easy_setopt(handle, option, "deflate,gzip");
			if (errornum != CURLE_OK) {
				puts(curl_easy_strerror(errornum));
				return errornum;
			}

			option = CURLOPT_WRITEFUNCTION;
			errornum = curl_easy_setopt(handle, option, &my_write);
			if (errornum != CURLE_OK) {
				puts(curl_easy_strerror(errornum));
				return errornum;
			}

			errornum = curl_easy_perform(handle);
			if (errornum != CURLE_OK) {
				puts(curl_easy_strerror(errornum));
				return errornum;
			}

			curl_easy_cleanup(handle);

			this->content = ::content;
			std::clog << this->content << std::endl;

			regex_content();


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
				this->task_r();
				this->task_x();
				this->task_w();

				sleep(1);
			} while (true);

			int ret = 0;
			return ret;
		}

		int task_r() {
			std::clog << ": " << __func__<< std::endl;

			std::list<page>::iterator i = urls.begin();
			while (i != urls.end()) {
				page p = *i;
				std::clog << ": " << p.url << std::endl;
				this->get_page(p);

				i++;
			}


			int ret = 0;
			return ret;
		}

		int task_x() {
			std::clog << ": " << __func__<< std::endl;

			std::list<page>::iterator i = urls.begin();
			while (i != urls.end()) {
				page p = *i;
				std::clog << ": " << p.url << std::endl;

				i++;
			}


			int ret = 0;
			return ret;
		}

		int task_w() {
			std::clog << ": " << __func__<< std::endl;

			std::list<page>::iterator i = urls.begin();
			while (i != urls.end()) {
				page p = *i;
				std::clog << ": " << p.url << std::endl;

				i++;
			}


			int ret = 0;
			return ret;
		}

		int get_page(page p) {

			p.i_want_to_get_this_content();

			int ret = p.content.size();
			return ret;
		}

		~spider() {
			std::clog << "Deconstruct: " << __func__<< std::endl;
		}
};

#endif
