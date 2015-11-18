/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(Transport *t, std::map<int, Transport*> *m, std::list<Transport*> *w, std::map<std::string, int> *interface) {
	int ret = 0;
	size_t length = 0;
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
	void *message = NULL;

	t->pr();
	do {
		if (t->get_rp() >= LENGTH) {
			for (i = 0; i < LENGTH; i++) {
				c = *((char *)t->get_rx() + i);
				if (c >= '0' && c <= '9') {
					length = length * 0x10 + (c - '0' + 0x00);
				} else if (c >= 'a' && c <= 'f') {
					length = length * 0x10 + (c - 'a' + 0x0a);
				} else if (c >= 'A' && c <= 'F') {
					length = length * 0x10 + (c - 'A' + 0x0a);
				} else {
					length = length * 0x10 + 0x00;
				}
			}
			plog(notice, "i8.length = 0x%08lx\n", length);
			width += LENGTH;
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + ID_LENGTH) {
			strncpy(source, (char *)t->get_rx() + width, ID_LENGTH);
			width += ID_LENGTH;
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + ID_LENGTH) {
			strncpy(destination, (char *)t->get_rx() + width, ID_LENGTH);
			width += ID_LENGTH;
		} else {
			/* Back to wait message. */
			break;
		}
		plog(notice, "source = \"%s\", destination = \"%s\", id = \"%s\"\n", source, destination, t->get_id().c_str());

		if (t->get_rp() >= width + MD5SUM_LENGTH) {
			memset(md5sum, 0, sizeof md5sum);
			memcpy(md5sum, (const void *)((char *)t->get_rx() + width), MD5SUM_LENGTH);
			width += MD5SUM_LENGTH;
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + length) {
			plog(info, "md5sum = \"%s\"\n", md5sum);
			message = malloc(length + 1);
			memset(message, 0, length + 1);
			memcpy(message, (const void *)((char *)t->get_rx() + width), length);
			ret = checksum(message, length, md5sum, digestname);
			free (message);
			if (ret == -1) {
				plog(warning, "Check fails.\n");
				t->clear_rx();
				/* Back to wait other message. */
				break;
			}
		} else {
			/* Back to wait message. */
			break;
		}

		if (strncmp(source, destination, sizeof source) == 0 && strlen(source) > 0) {
			// update or refresh interface
			interface->erase(t->get_id());

			t->set_id(source);
			t->set_alive(true);
			interface->insert(std::make_pair(t->get_id(), t->get_fd()));
			if (t->get_rp() >= width) {
				plog(notice, "Echo\n");
				t->set_wx(t->get_rx(), t->get_rp());
				t->clear_rx();
				w->push_back(t);
			} else {
				break;
			}
		} else if (t->get_rp() >= width + length) {
			plog(notice, "Message complete, width + length = 0x%lx, rp = 0x%lx\n", width + length, t->get_rp());
			id = source;
			if (t->get_id() != id) {
				plog(warning, "Non self id.\n");
				t->clear_rx();
				break;
			}

			int fdx = 0;
			Transport *tx = NULL;
			id = destination;

			std::map<std::string, int>::iterator ie = interface->find(id);
			if (ie != interface->end()) {
//				plog(debug, "ie->first.c_str() = %s, ie->second = %d\n", ie->first.c_str(), ie->second);
				fdx = ie->second;
			} else {
				plog(info, "Back to wait id = \"%s\"\n", destination);
				break;
			}

			std::map<int, Transport*>::iterator im = m->find(fdx);
			if (im != m->end()) {
//				plog(debug, "Found, first = %d, second = %p\n", im->first, im->second);
				tx = im->second;
			} else {
				plog(info, "Back to wait id = \"%s\"\n", destination);
				break;
			}

			if (t->get_rp() >= width) {
				tx->set_wx(t->get_rx(), (width + length));
				memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (width + length)), t->get_rp() - (width + length));
				t->set_rp(t->get_rp() - (width + length));
				w->push_back(tx);
			} else {
				break;
			}
		}
	} while (false);
	return ret;
}

int checksum(const void *ptr, size_t size, char *md_value_0, char *digestname) {
	int ret = 0;
	return ret;

	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	size_t md_len, i;

	OpenSSL_add_all_digests();
	md = EVP_get_digestbyname(digestname);
	if (!md) {
		plog(error, "Unknown message digest %s\n", digestname);
		return -1;
	}

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, ptr, size);
	EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *)&md_len);
	EVP_MD_CTX_destroy(mdctx);

	for (i = 0; i < size; i++) plog(debug, "%c", *(char *)((char *)ptr + i));
	puts("");

	puts("Original Digest is: {");
	for (i = 0; i < strlen(md_value_0); i++) plog(debug, "%c", md_value_0[i]);
	puts("}");

	puts("Computed Digest is: {");
	for (i = 0; i < md_len; i++) plog(debug, "%02x", md_value[i]);
	puts("}");

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
			ret = -1;
			break;
		}
	}
	return ret;
}
