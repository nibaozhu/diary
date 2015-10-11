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
	size_t width = 0;
	char md5sum[MD5SUM_LENGTH + 1];
	char digestname[] = "md5";
	char source[ID_LENGTH + 1];
	char destination[ID_LENGTH + 1];
	memset(source, 0, sizeof source);
	memset(destination, 0, sizeof destination);
	int i = 0;
	char c = 0;
	std::string id;
	static std::map<std::string, int> *interface = new std::map<std::string, int>(); // maybe should use multimap
	void *message = NULL;

	plog(debug, "rx = %p, rp = 0x%lx\n", t->get_rx(), t->get_rp());
	t->pr();
	do {

		if (t->get_rp() >= LENGTH) {
			for (i = 0; i < LENGTH; i++) {
				c = *((char*)t->get_rx() + i);
				if (c >= '0' && c <= '9') {
					length = length * 0x10 + (c - '0' + 0x00);
				} else if (c >= 'a' && c <= 'f') {
					length = length * 0x10 + (c - 'a' + 0x0a);
				} else if (c >= 'A' && c <= 'F') {
					length = length * 0x10 + (c - 'A' + 0x0a);
				}
			}
			plog(debug, "i8.length = 0x%08x\n", length);
			width += LENGTH;
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + ID_LENGTH) {
			strncpy(source, (char*)t->get_rx() + width, ID_LENGTH);
			plog(debug, "source = \"%s\", id = \"%s\"\n", source, t->get_id().c_str());
			width += ID_LENGTH;
		} else {
			/* Back to wait message. */
			/* Wait message */
			break;
		}

		if (t->get_rp() >= width + ID_LENGTH) {
			strncpy(destination, (char*)t->get_rx() + width, ID_LENGTH);
			plog(debug, "destination = \"%s\", id = \"%s\"\n", destination, t->get_id().c_str());
			width += ID_LENGTH;
		} else {
			/* Wait message */
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + MD5SUM_LENGTH) {
			memset(md5sum, 0, sizeof md5sum);
			memcpy(md5sum, (const void *)((char *)t->get_rx() + width), MD5SUM_LENGTH);
			plog(debug, "i2.md5sum = \"%s\"\n", md5sum);
			width += MD5SUM_LENGTH;
		} else {
			/* Wait message */
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + length) {
			message = malloc(length + 1);
			memset(message, 0, sizeof message);
			memcpy(message, (const void *)((char *)t->get_rx() + width), length);
			ret = checksum(message, length, md5sum, digestname);
			if (ret == -1) {
				plog(debug, "checksum FAIL\n");
				t->clear_rx();
				/* Back to wait other message. */
				break;
			}
		} else {
			/* Wait message */
			/* Back to wait message. */
			break;
		}

		if (strncmp(source, destination, sizeof source) == 0 && strlen(source) > 0) {
			plog(debug, "Echo.\n");
			interface->erase(t->get_id());
			t->set_id(source);
			(*interface)[t->get_id()] = t->get_fd();
			t->set_id(source);
			if (t->get_rp() >= width) {
				t->set_wx(t->get_rx(), t->get_rp());
				t->clear_rx();
				w->push(t);
			} else {
				break;
			}
		} else if (t->get_rp() >= width + length) {
			plog(debug, "Message complete, width + length = 0x%lx, rp = 0x%lx\n", width + length, t->get_rp());
			id = destination;
			Transport *t2 = (*m)[(*interface)[id]];
			if (t2 == NULL) {
				plog(debug, "Back to wait id = \"%s\"\n", destination);
				break;
			}

			if (t->get_rp() >= width) {
				t2->set_wx(t->get_rx(), (width + length));
				memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (width + length)), t->get_rp() - (width + length));
				t->set_rp(t->get_rp() - (width + length));
				w->push(t2);
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
	int md_len, i;

	OpenSSL_add_all_digests();
	md = EVP_get_digestbyname(digestname);
	if (!md) {
		plog(debug, "Unknown message digest %s\n", digestname);
		return -1;
	}

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, ptr, size);
	EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *)&md_len);
	EVP_MD_CTX_destroy(mdctx);

#if 0
	for (i = 0; i < size; i++) plog(debug, "%c", *(char *)((char *)ptr + i));
	puts("");

	puts("Original Digest is: {");
	for (i = 0; i < strlen(md_value_0); i++) plog(debug, "%c", md_value_0[i]);
	puts("}");

	puts("Computed Digest is: {");
	for (i = 0; i < md_len; i++) plog(debug, "%02x", md_value[i]);
	puts("}");
#endif

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
			puts("Warning: CHECKSUM NOT EQUAL!\n");
			ret = -1;
			break;
		}
	}
	return ret;
}
