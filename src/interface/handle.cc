/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"

/*
 * %t: ....
 */
int handle(Transport *t, std::map<int, Transport*> *m, std::queue<Transport*> *w) {
	int ret = 0;
	unsigned int length = 0;
	int width = 0;
	char md5sum[MD5SUM_LENGTH + 1];
	char digestname[] = "md5";
	int identification0 = 0;
	int identification1 = 0;
	int i = 0;
	char c = 0;
	static std::map<int, int> *interface = new std::map<int, int>(); // should use multimap
	void *message = NULL;

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
			/* WARNING: reset memory */
			printf("Drop the message, reset memory.\n");
			t->clear_rx();
			// return ret;
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
			/* WARNING: reset memory */
			printf("Drop the message, reset memory.\n");
			t->clear_rx();
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
			/* WARNING: reset memory */
			printf("Drop the message, reset memory.\n");
			t->clear_rx();
			break;
		}

		if (t->get_rp() >= width + MD5SUM_LENGTH) {
			memset(md5sum, 0, sizeof md5sum);
			memcpy(md5sum, t->get_rx() + width, MD5SUM_LENGTH);
			printf("i2.md5sum = {%s}\n", md5sum);
			width += MD5SUM_LENGTH;
		} else {
			/* WARNING: reset memory */
			printf("Drop the message, reset memory.\n");
			t->clear_rx();
			break;
		}

		if (t->get_rp() >= width + length) {
			message = malloc(length + 1);
			memset(message, 0, sizeof message);
			memcpy(message, t->get_rx() + width, length);
			ret = checksum(message, length, md5sum, digestname);
			if (ret == -1) {
				/* WARNING: reset memory */
				printf("Drop the message, reset memory.\n");
				t->clear_rx();
				// return ret;
				break;
			}
		}

		if (identification0 == identification1 && identification0 != 0) {

			/* Remove Old identification. */
			interface->erase(t->get_identification());

			/* Insert into interface. */
			(*interface)[identification0] = t->get_fd();

			/* This is Register Message. */
			t->set_identification(identification0);

			printf("Echo.\n");
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
		} else if (t->get_rp() >= width + length) {
			printf("Message complete, %d, %d\n", width + length, t->get_rp());
			/* This is complete Message. */

			/* Find transport, according to identification1. */
			// temporarily ignore
			// t->set_identification(identification1); // maybe wrong

			// fd = (*interface)[identification1];
			Transport *t2 = (*m)[(*interface)[identification1]];
			if (t2 == NULL) {
				printf("Waiting identification = 0x%08x\n", identification1);
				break; // maybe wrong
			}

			if (t->get_rp() >= width) {
				// t->set_wx(t->get_rx(), t->get_rp());
				t2->set_wx(t->get_rx(), (width + length));

				/* WARNING: reset memory */
				// t->clear_rx();

				memmove(t->get_rx(), t->get_rx() + (width + length), t->get_rp() - (width + length));
				t->set_rp(t->get_rp() - (width + length));

				/* WARNING: push w */
				w->push(t2);

				// t->set_wp(t->get_wp() + width);
			} else {
				break;
			}
		}
	} while (0);
	return ret;
}

int checksum(const void *ptr, int size, char *md_value_0, char *digestname) {
	int ret = 0;
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	char md_value_x;
	int md_len, i;

	OpenSSL_add_all_digests();
	md = EVP_get_digestbyname(digestname);
	if (!md) {
		printf("Unknown message digest %s\n", digestname);
		return -1;
	}

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, ptr, size);
	EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *)&md_len);
	EVP_MD_CTX_destroy(mdctx);

#ifdef D
	for (i = 0; i < size; i++) printf("%c", *(char *)((char *)ptr + i));
	puts("");
#endif

	printf("Original Digest is: {");
	for (i = 0; i < strlen(md_value_0); i++) printf("%c", md_value_0[i]);
	printf("}\n");

	printf("Computed Digest is: {");
	for (i = 0; i < md_len; i++) printf("%02x", md_value[i]);
	printf("}\n");

	for (i = 0; i < md_len; i++) {

		if (md_value_0[2 * i] >= '0' && md_value_0[2 * i] <= '9') {
			md_value_0[2 * i] = md_value_0[2 * i] - '0';
		} else if (md_value_0[2 * i] >= 'a' && md_value_0[2 * i] <= 'f') {
			md_value_0[2 * i] = md_value_0[2 * i] - 'a' + 0x0a;
		} else if (md_value_0[2 * i] >= 'A' && md_value_0[2 * i] <= 'F') {
			md_value_0[2 * i] = md_value_0[2 * i] - 'A' + 0x0a;
		}

		if (md_value_0[2 * i + 1] >= '0' && md_value_0[2 * i + 1] <= '9') {
			md_value_0[2 * i + 1] = md_value_0[2 * i + 1] - '0';
		} else if (md_value_0[2 * i + 1] >= 'a' && md_value_0[2 * i + 1] <= 'f') {
			md_value_0[2 * i + 1] = md_value_0[2 * i + 1] - 'a' + 0x0a;
		} else if (md_value_0[2 * i + 1] >= 'A' && md_value_0[2 * i + 1] <= 'F') {
			md_value_0[2 * i + 1] = md_value_0[2 * i + 1] - 'A' + 0x0a;
		}

		if (md_value[i] != md_value_0[2 * i] * 0x10 + md_value_0[2 * i + 1]) {
			printf("Not equal");
			ret = -1;
			break;
		}
	}
	return ret;
}
