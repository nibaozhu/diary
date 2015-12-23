/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *interface, Transport* t) {
	int ret = 0;

	t->pr();
	do {
		uint32_t length = 0;
		uint32_t id[2] = {0, 0};
		char temporarily[sizeof (uint32_t)];
		memset(temporarily, 0, sizeof temporarily);

		if (t->get_rp() >= sizeof (uint32_t)) {
			memcpy(temporarily, t->get_rx(), sizeof (uint32_t));
			length = atoi(temporarily);
			length = ntohl(length);
			plog(info, "length = 0x%x\n", length);
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= 3 * sizeof (uint32_t) + length) {
			memcpy(temporarily, (char *)t->get_rx() + sizeof (uint32_t), sizeof (uint32_t));
			id[0] = atoi(temporarily);
			id[0] = ntohl(id[0]);

			memcpy(temporarily, (char *)t->get_rx() + 2 * sizeof (uint32_t), sizeof (uint32_t));
			id[1] = atoi(temporarily);
			id[1] = ntohl(id[1]);
		} else {
			/* Back to wait message. */
			break;
		}

		plog(notice, "length = 0x%x, id = {0x%x(0x%x), 0x%x}\n", length, id[0], t->get_id(), id[1]);
		if (0) {
			t->clear_rx(); /* Fixed: need reset connection */
			/* Back to wait other message. */
			break;
		}

		if (id[0] == id[1] && id[0] != 0) {
			t->set_alive(true);

			interface->erase(t->get_id());
			t->set_id(id[0]);

			plog(notice, "Echo, id = 0x%x\n", id[0]);
			t->set_wx(t->get_rx(), t->get_rp());
			t->clear_rx();
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

			std::map<uint32_t, int>::iterator ie = interface->find(id[1]);
			if (ie != interface->end()) {
#if 0
				plog(debug, "ie->first = %u, ie->second = %d\n", ie->first, ie->second);
#endif
				fx = ie->second;
			} else {
				plog(info, "Back to wait id(0x%x)\n", id[1]);
				break;
			}

			std::map<int, Transport*>::iterator im = m->find(fx);
			if (im != m->end()) {
#if 0
				plog(debug, "Found, first = %d, second = %p\n", im->first, im->second);
#endif
				tx = im->second;
			} else {
				plog(info, "Back to wait id(0x%x)\n", id[1]);
				break;
			}

			tx->set_wx(t->get_rx(), (3 * sizeof (uint32_t) + length));
			memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (3 * sizeof (uint32_t) + length)), t->get_rp() - (3 * sizeof (uint32_t) + length));
			t->set_rp(t->get_rp() - (3 * sizeof (uint32_t) + length));
			w->push_back(tx);
		}
	} while (true);
	return ret;
}
