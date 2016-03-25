/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(Transport* t, std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *__m) {
	int ret = 0;
	int i = 0;
	size_t length = 0;

	t->pr();
	do {
		/* MESSAGE HEAD */
		plog(notice, "[+] Transaction(%d) Begin: length = 0x%lx,w = %p, m = %p, __m = %p\n", i, length, w, m, __m);

/***************************
 * http, json, mysql
 * *************************/




		/* MESSAGE BODY */
		plog(info, "[x] Transaction(%d) Passed.\n", i++);

		break;
	} while (true);

	if (t->get_rp() != 0) {
		plog(info, "[!] Transaction(%d) Cancel! length = 0x%lx, t->rp = 0x%lx\n", i, length, t->get_rp());
	}
	return ret;
}
