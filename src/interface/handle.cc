/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"

/*
 * %t: ....
 */
int handle(Transport *t, std::queue<Transport*> *w) {
	int ret = 0;
	uint32_t length = 0;
	int width = 0;
	char md5sum[MD5SUM_LENGTH + 1];
	char identification0[4 + 1];
	char identification1[4 + 1];
	uint32_t id0;
	uint32_t id1;

	printf("rx = %p, rp = %d\n", t->get_rx(), t->get_rp());
	t->pr();
	while (true) {

		if (t->get_rp() >= 4) {
			memcpy(&length, t->get_rx(), sizeof (uint32_t));
			length = ntohl(length);
			printf("i2.length = 0x%08x\n", length);
			width = sizeof (uint32_t);
		} else {
			break;
		}

		if (t->get_rp() >= width + MD5SUM_LENGTH) {
			memset(md5sum, 0, sizeof md5sum);
			memcpy(md5sum, t->get_rx() + width, width + MD5SUM_LENGTH);
			printf("i2.md5sum = {%s}\n", md5sum);
			width += MD5SUM_LENGTH;
		} else {
			break;
		}

		if (t->get_rp() >= width + 4) {
			memset(identification0, 0, sizeof identification0);
			memcpy(identification0, t->get_rx() + width, sizeof (uint32_t));
			printf("identification0 = 0x%08x, identification = 0x%08x\n", (uint32_t)*identification0, t->get_identification());
			width += sizeof (uint32_t);
		} else {
			break;
		}

		if (t->get_rp() >= width + 4) {
			memset(identification1, 0, sizeof identification1);
			memcpy(identification1, t->get_rx() + width, sizeof (uint32_t));
			printf("identification1 = 0x%08x, identification = 0x%08x\n", (uint32_t)*identification1, t->get_identification());
			width += sizeof (uint32_t);
		} else {
			break;
		}

//		message = malloc(length + 1);
//		memset(message, 0, sizeof message);
//		memcpy(message, t->get_rx() + width, sizeof length);
//		ret = md5sum(message, length);
		if (ret == -1) {
			return ret;
		}

		id0 = ntohl((uint32_t)*identification0);
		id1 = ntohl((uint32_t)*identification1);
		if (id0 == id1 && id0 != 0) {
			printf("Echo\n");
			/* This is Register Message. */
			t->set_identification(id0);

			if (t->get_rp() >= width) {
				t->set_wx(t->get_rx(), t->get_rp());

				/* WARNING: reset memory */
				t->clear_rx();

				/* WARNING: push w */
				w->push(t);

				// t->set_wp(t->get_wp() + width);
			} else {
				break;
			}

			;;;;;;;;;;;;;;;;;;;;
			// maybe memory move
			;;;;;;;;;;;;;;;;;;;;
		} else if (length < t->get_rp()) {
			printf("Message complete\n");
			/* This is complete Message. */

			/* Find transport, according to identification1. */
			// temporarily ignore
			t->set_identification(id1); // maybe wrong

			if (t->get_rp() >= width) {
				t->set_wx(t->get_rx(), t->get_rp());

				/* WARNING: reset memory */
				t->clear_rx();

				/* WARNING: push w */
				w->push(t);

				// t->set_wp(t->get_wp() + width);
			} else {
				break;
			}

			;;;;;;;;;;;;;;;;;;;;
			// maybe memory move
			;;;;;;;;;;;;;;;;;;;;
		}

		break;
	}

	return ret;
}

int md5sum(const char *ptr, int size) {
	int ret = 0;

	printf("ptr = %p, size =%d\n", ptr, size);

	return ret;
}
