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

		message = malloc(length + 1);
		memset(message, 0, sizeof message);
		memcpy(message, t->get_rx() + width, sizeof length);
		ret = checksum((char *)message, length, md5sum, digestname);
		if (ret == -1) {
			/* WARNING: reset memory */
			printf("Drop the message, reset memory.\n");
			t->clear_rx();
			// return ret;
			break;
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
		} else if (t->get_rp() >= LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH + length) {
			printf("Message complete, %d, %d\n", length + LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH, t->get_rp());
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
				t2->set_wx(t->get_rx(), LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH + length);

				/* WARNING: reset memory */
				// t->clear_rx();
				memmove(t->get_rx(), 
					t->get_rx() + LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH + length, 
					t->get_rp() - LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH + length);
				t->set_rp(t->get_rp() - LENGTH + IDENTIFICATION_LENGTH * 2 + MD5SUM_LENGTH + length);

				/* WARNING: push w */
				w->push(t2);

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

int checksum(const char *ptr, int size, char *md_value_0, char *digestname) {
	int ret = 0;
	printf("ptr = %p, size =%d\n", ptr, size);

	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	int md_len, i;

	OpenSSL_add_all_digests();
	md = EVP_get_digestbyname(digestname);
	if(!md) {
		printf("Unknown message digest %s\n", digestname);
		return -1;
	}

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, ptr, size);
	EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *)&md_len);
	EVP_MD_CTX_destroy(mdctx);

	printf("Original Digest is: {");
	for(i = 0; i < strlen(md_value_0); i++) printf("%c", md_value_0[i]);
	printf("}\n");

	printf("Computed Digest is: {");
	for(i = 0; i < md_len; i++) printf("%02x", md_value[i]);
	printf("}\n");

	if (memcmp(md_value_0, md_value, md_len) != 0) {
		printf("Not Equal!\n");
		ret = -1;
	}

	return ret;
}
