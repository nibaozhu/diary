
#include <iostream>

#include <list>

#include <unistd.h>


class page {
	public:
		std::string url;
		std::string host;
		std::string ip;

		std::string title;
};

class spider {
	public:
		page root;
		std::list<page> urls;

		spider() {
			std::clog << "Construct: " << __func__<< std::endl;
			this->root.url = "http://www.sohu.com/";
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

		int get_page(std::string page) {

			int ret = 0;
			return ret;
		}


		~spider() {
			std::clog << "Deconstruct: " << __func__<< std::endl;
		}
};
