/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(Transport* t, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *__m) {
	int ret = 0;
	int i = 0;

	// XXX: should add delete variable_value
	std::map<std::string, std::string> *variable_value = new std::map<std::string, std::string>();

	t->pr();
	do {
		size_t length_request = 0;
		size_t length_method = 0;
		size_t length_location = 0;
		size_t length_variable = 0;
		size_t length_value = 0;

		char *position_method = NULL;
		char *position_rn = NULL;
		char *position_location = NULL;
		char *position_request = NULL;

		std::string request_location("");
		std::string variable("");
		std::string value("");

		bool is_get = false;
		bool is_post = false;

		/* MESSAGE HEAD */
		plog(notice, "[+] Transaction(%d) Begin: length = 0x%lx,w = %p, m = %p, __m = %p\n", i, length_request, w, m, __m);

/***************************
 * http, json, mysql
 * *************************/
		/* GET, POST */
		const char *REQUEST_GET = "GET ";
		const char *REQUEST_POST = "POST ";
		const char *RN = "\r\n";
		const char *RNRN = "\r\n\r\n";
		const char *QUESTION_MARK = "?";
		const char *EQUAL_SIGN = "=";
		const char *AND_SIGN = "&";
		const char *BLANK = " ";

		position_request = strstr((char*)t->get_rx(), RNRN);
		if (position_request == NULL) {
			/* message is not completed */
			break;
		} else {
			length_request = position_request - (char*)t->get_rx() + strlen(RNRN);
		}

		position_method = strstr((char*)t->get_rx(), REQUEST_GET);
		if (position_method != (char*)t->get_rx()) {
			position_method = strstr((char*)t->get_rx(), REQUEST_POST);
			if (position_method != (char*)t->get_rx()) {
				/* invalid message, discard. */
				t->clear_rx();
				break;
			} else {
				is_post = true;
				length_method = strlen(REQUEST_POST);
			}
		} else {
			is_get = true;
			length_method = strlen(REQUEST_GET);
		}

		position_rn = strstr((char*)t->get_rx() + length_method, RN);
		if (position_rn == NULL) {
			/* invalid message, discard. */
			t->clear_rx();
			break;
		} else {
			// XXX: should add url-decode for CJK-language

			/* location, {?, variable, =, value{, ..., &, variable, =, value}} */
			position_location = strstr((char*)t->get_rx() + length_method, QUESTION_MARK);
			if (position_location == NULL) {
				position_location = strstr((char*)t->get_rx() + length_method, BLANK);
				if (position_location == NULL) {
					/* invalid message, discard. */
					t->clear_rx();
					break;
				} else {
					length_location = position_location - ((char*)t->get_rx() + length_method);
					request_location.assign((char*)t->get_rx() + length_method, length_location);
				}
			} else {
				length_location = position_location - ((char*)t->get_rx() + length_method);
				request_location.assign((char*)t->get_rx() + length_method, length_location);

				// XXX: variable=value
				parse_variable_value(variable_value, (char*)t->get_rx() + length_method + length_location, length_request - (length_method + length_location));
			}


			/* business code begin */
			business(t, request_location, variable_value);
			/* business code end */
		}

		memmove(t->get_rx(), (const void *)((char *)t->get_rx() + length_request), t->get_rp() - length_request);
		t->set_rp(t->get_rp() - length_request);

		/* MESSAGE BODY */
		plog(info, "[x] Transaction(%d) Passed.\n", i++);

		break;
	} while (true);

	if (t->get_rp() != 0) {
		plog(info, "[!] Transaction(%d) Cancel! t->rp = 0x%lx\n", i, t->get_rp());
	}
	return ret;
}


void parse_variable_value(std::map<std::string, std::string> *variable_value, const char* src, size_t size) {
	plog(debug, "variable_value = %p, src = %p, size = 0x%lx\n", variable_value, src, size);

	return ;
}

void business(Transport *t, std::string request_location, std::map<std::string, std::string> *variable_value) {
	plog(debug, "t = %p, request_location = %s, variable_value = %p\n", t, request_location.c_str(), variable_value);

	int command = rand() % 4;
	switch (command) {
		case INSERT_PRODUCT:
			;;;
			break;
		case DELETE_PRODUCT:
			;;;
			break;
		case SELECT_PRODUCT:
			;;;
			break;
		case UPDATE_PRODUCT:
			;;;
			break;
		default:
			;;;
	}

	plog(debug, "command = %d\n", command);
	return ;
}

