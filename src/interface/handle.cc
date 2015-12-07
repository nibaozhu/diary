/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"

int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *interface, Transport* t) {
	int ret = 0;
	char source[ID_LENGTH + 1];
	char destination[ID_LENGTH + 1];
	memset(source, 0, sizeof source);
	memset(destination, 0, sizeof destination);
	std::string id;

	t->pr();
	do {
		size_t length = 0;
		size_t head = 0;

		if (t->get_rp() >= LENGTH) {
			size_t i = 0;
			for ( ; i < LENGTH; i++) {
				char c = *((char *)t->get_rx() + i);
				if (c >= '0' && c <= '9') {
					length = length * 0x10 + (c - '0' + 0x00);
				} else if (c >= 'a' && c <= 'f') {
					length = length * 0x10 + (c - 'a' + 0x0a);
				} else if (c >= 'A' && c <= 'F') {
					length = length * 0x10 + (c - 'A' + 0x0a);
				} else {
					plog(debug, "delimiter is '%c'\n", c);
					break;
				}
			}
			length++; /* Last delimiter. */
			head += i + 1;
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= head + ID_LENGTH) {
			strncpy(source, (char *)t->get_rx() + head, ID_LENGTH);
			head += ID_LENGTH;
			head++; /* delimiter */
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= head + ID_LENGTH) {
			strncpy(destination, (char *)t->get_rx() + head, ID_LENGTH);
			head += ID_LENGTH;
			head++; /* delimiter */
		} else {
			/* Back to wait message. */
			break;
		}

		plog(notice, "length = 0x%lx, source = '%s'(id = '%s'), destination = '%s'\n", length, source, t->get_id().c_str(), destination);
		if (checkid((void *)source, strlen(source)) == -1 || checkid((void *)destination, strlen(destination)) == -1) {
			t->clear_rx(); /* Fixed: need reset connection */
			/* Back to wait other message. */
			break;
		}

		if (strncmp(source, destination, sizeof source) == 0 && strlen(source) > 0) {
			t->set_alive(true);
			plog(notice, "Echo\n");
			t->set_wx(t->get_rx(), t->get_rp());
			t->clear_rx();
			w->push_back(t);
		} else if (t->get_rp() >= head + length) {
			plog(notice, "Message completed, head(0x%lx) + length(0x%lx) = 0x%lx\n", head, length, head + length);
			id = source;
			if (t->get_id() != id) {
				plog(warning, "Non self id.\n");
				t->clear_rx();
				break;
			}

			int fx = 0;
			Transport* tx = NULL;
			id = destination;

			std::map<std::string, int>::iterator ie = interface->find(id);
			if (ie != interface->end()) {
#if 0
				plog(debug, "ie->first.c_str() = %s, ie->second = %d\n", ie->first.c_str(), ie->second);
#endif
				fx = ie->second;
			} else {
				plog(info, "Back to wait id = '%s'\n", destination);
				break;
			}

			std::map<int, Transport*>::iterator im = m->find(fx);
			if (im != m->end()) {
#if 0
				plog(debug, "Found, first = %d, second = %p\n", im->first, im->second);
#endif
				tx = im->second;
			} else {
				plog(info, "Back to wait id = '%s'\n", destination);
				break;
			}

			tx->set_wx(t->get_rx(), (head + length));
			memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (head + length)), t->get_rp() - (head + length));
			t->set_rp(t->get_rp() - (head + length));
			w->push_back(tx);
		}
	} while (true);
	return ret;
}

int checkid(const void *ptr, size_t size) {
	int ret = 0;
	for (size_t i = 0; i < size; i++) {
		int c = *((char *)ptr + i);
		/* The values returned are non-zero if the character c falls into the tested class, and a zero value if not. */
		/* Checks for a hexadecimal digits, that is, one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F. */
		if (isxdigit(c) == 0) {
			ret = -1;
			break;
		}
	}
	return ret;
}

