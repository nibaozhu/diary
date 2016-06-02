/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *__m, Transport* t) {
	int ret = 0;
	int i = 0;
	uint32_t length = 0;
	const char *iterator_position = (const char *)t->get_rx();

	t->pr();
	do {
		/* TODO: HTTP PROTOCOL PROXY */

		/* HTTP HEADER */
		const char *haystack = iterator_position;
		const char *needle = (const char *)"\r\n\r\n";
		const char *occurrence_RNRN = strstr(haystack, needle);
		plog(debug, "occurrence_RNRN(\\r\\n\\r\\n) = %p\n", occurrence_RNRN);
		if (occurrence_RNRN) {

			size_t http_header_length = occurrence_RNRN - haystack + 1;
			char *http_header = (char *)malloc(http_header_length + 1);
			memset(http_header, 0, http_header_length + 1);
			memcpy(http_header, t->get_rx(), http_header_length);
			plog(info, "--- HTTP HEADER ---\n%s\n------------------------------------------- HTTP ----------\n", http_header);

			haystack = (const char *)http_header;
			needle = (const char *)"Host: ";
			const char *occurrence_H = strstr(haystack, needle);
			if (occurrence_H) {

			} else {
				break;
			}

			haystack = occurrence_H;
			needle = (const char *)"\r\n";
			const char *occurrence_line_ending = strstr(haystack, needle);
			if (occurrence_line_ending) {

			} else {
				break;
			}

			size_t http_header_host_length = occurrence_line_ending - occurrence_H - strlen("Host: ");
			char *http_header_host = (char *)malloc(http_header_host_length + 1);
			memset(http_header_host, 0, http_header_host_length + 1);
			memcpy(http_header_host, occurrence_H + strlen("Host: "), http_header_host_length);

			plog(debug, "Host: %s\n", http_header_host);

			int connect_sock = 0;
			ret = connect_to_host(http_header_host, connect_sock);
			if (ret == -1) {
				continue;
			}


			free(http_header_host);
			http_header_host = NULL;

			free(http_header);
			http_header = NULL;

			iterator_position = occurrence_RNRN + strlen("\r\n\r\n");
		} else {
			break;
		}

#if 0
		if (t->get_rp() >= 3 * sizeof (uint32_t)) {
			memcpy(&length, t->get_rx(), sizeof (uint32_t));

		} else {
			/* Back to wait message. */
			break;
		}

		/* MESSAGE BODY */
		if (id[0] == id[1] && id[0] != 0 && t->get_rp() >= (3 * sizeof (uint32_t) + length)) {
			if (t->get_id() != id[0]) {
				if (t->get_id() != 0) {
					__m->erase(t->get_id());
					plog(warning, "Erase pair{id(0x%x), fd(%d)} from __m(%p)\n", t->get_id(), t->get_fd(), __m);
				}

				t->set_id(id[0]);
				__m->insert(std::make_pair(t->get_id(), t->get_fd()));
				plog(info, "Insert pair{id(0x%x), fd(%d)} into __m(%p)\n", t->get_id(), t->get_fd(), __m);
				plog(notice, "Welcome and Echo. id = 0x%x\n", id[0]);
			}

			t->set_wx(t->get_rx(), 3 * sizeof (uint32_t) + length);
			memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (3 * sizeof (uint32_t) + length)), t->get_rp() - (3 * sizeof (uint32_t) + length));
			t->set_rp(t->get_rp() - (3 * sizeof (uint32_t) + length));
			w->push_back(t);
		} else if (t->get_rp() >= (3 * sizeof (uint32_t) + length)) {
			plog(notice, "Message completed(=0x%x).\n", (unsigned int)(3 * sizeof (uint32_t)) + length);
			if (t->get_id() != id[0]) {
				plog(warning, "id = 0x%x, Non self(0x%x).\n", id[0], t->get_id());
				t->clear_rx();
				break;
			}

			int fx = 0;
			Transport* tx = NULL;

			std::map<uint32_t, int>::iterator ie = __m->find(id[1]);
			if (ie != __m->end()) {
				fx = ie->second;
			} else {
				plog(info, "Back to wait id(0x%x)\n", id[1]);
				break;
			}

			std::map<int, Transport*>::iterator im = m->find(fx);
			if (im != m->end()) {
				tx = im->second;
			} else {
				plog(info, "Back to wait id(0x%x)\n", id[1]);
				break;
			}

			tx->set_wx(t->get_rx(), (3 * sizeof (uint32_t) + length));
			memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (3 * sizeof (uint32_t) + length)), t->get_rp() - (3 * sizeof (uint32_t) + length));
			t->set_rp(t->get_rp() - (3 * sizeof (uint32_t) + length));
			w->push_back(tx);
		} else {
			/* Back to wait message. */
			break;
		}

		plog(info, "[x] Transaction(%d) Passed.\n", i++);
#endif
	} while (true);

	if (t->get_rp() != 0) {
		plog(info, "[!] Transaction(%d) Cancel! length = 0x%x, t->rp = 0x%lx\n", i, length, t->get_rp());
	}
	return ret;
}
