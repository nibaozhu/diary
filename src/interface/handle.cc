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
	unsigned int length = 0;
	int width = 0;
	char md5sum[MD5SUM_LENGTH + 1];
	int identification0;
	int identification1;
	int i = 0;
	char c = 0;

	printf("rx = %p, rp = %d\n", t->get_rx(), t->get_rp());
	t->pr();
	do {

		if (t->get_rp() >= LENGTH) {
			for (i = 0; i < LENGTH; i++) {
				c = *(char*)(t->get_rx() + i);
				if (c >= '0' && c <= '9') {
					length = length * 0x10 + (c - '0' + 0x00);
				} else if (c >= 'a' && c <= 'f') {
					length = length * 0x10 + (c - 'a' + 0x0a);
				} else if (c >= 'A' && c <= 'F') {
					length = length * 0x10 + (c - 'A' + 0x0a);
				}
			}
			printf("i2.length = 0x%08x\n", length);
			width += LENGTH;
		} else {
			break;
		}

		if (t->get_rp() >= width + IDENTIFICATION_LENGTH) {
			for (i = 0; i < IDENTIFICATION_LENGTH; i++) {
				c = *(char*)(t->get_rx() + width + i);
				if (c >= '0' && c <= '9') {
					identification0 = identification0 * 0x10 + (c - '0' + 0x00);
				} else if (c >= 'a' && c <= 'f') {
					identification0 = identification0 * 0x10 + (c - 'a' + 0x0a);
				} else if (c >= 'A' && c <= 'F') {
					identification0 = identification0 * 0x10 + (c - 'A' + 0x0a);
				}
			}
			printf("identification0 = 0x%08x, identification = 0x%08x\n", identification0, t->get_identification());
			width += IDENTIFICATION_LENGTH;
		} else {
			break;
		}

		if (t->get_rp() >= width + IDENTIFICATION_LENGTH) {
			for (i = 0; i < IDENTIFICATION_LENGTH; i++) {
				c = *(char*)(t->get_rx() + width + i);
				if (c >= '0' && c <= '9') {
					identification1 = identification1 * 0x10 + (c - '0' + 0x00);
				} else if (c >= 'a' && c <= 'f') {
					identification1 = identification1 * 0x10 + (c - 'a' + 0x0a);
				} else if (c >= 'A' && c <= 'F') {
					identification1 = identification1 * 0x10 + (c - 'A' + 0x0a);
				}
			}
			printf("identification1 = 0x%08x, identification = 0x%08x\n", identification1, t->get_identification());
			width += IDENTIFICATION_LENGTH;
		} else {
			break;
		}

		if (t->get_rp() >= width + MD5SUM_LENGTH) {
			memset(md5sum, 0, sizeof md5sum);
			memcpy(md5sum, t->get_rx() + width, MD5SUM_LENGTH);
			printf("i2.md5sum = {%s}\n", md5sum);
			width += MD5SUM_LENGTH;
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

		if (identification0 == identification1 && identification0 != 0) {
			printf("Echo\n");
			/* This is Register Message. */
			t->set_identification(identification0);

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
		} else if (length + LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH < t->get_rp()) {
			printf("Message complete, %d, %d\n", length + LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH, t->get_rp() );
			/* This is complete Message. */

			/* Find transport, according to identification1. */
			// temporarily ignore
			t->set_identification(identification1); // maybe wrong

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
	} while (0);

	return ret;
}

int md5sum(const char *ptr, int size) {
	int ret = 0;

	printf("ptr = %p, size =%d\n", ptr, size);

	return ret;
}
