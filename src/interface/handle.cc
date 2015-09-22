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
	char identification0[4];
	char identification1[4];

	printf("rx = %p, rp = %d\n", t->get_rx(), t->get_rp());
	t->pr();
	while (true) {

		memcpy(&length, t->get_rx(), sizeof (uint32_t));
		length = ntohl(length);
		printf("i2.length = 0x%08x\n", length);
		width = sizeof (uint32_t);

		memset(md5sum, 0, sizeof md5sum);
		memcpy(md5sum, t->get_rx() + width, width + MD5SUM_LENGTH);
		printf("i2.md5sum = %-32s\n", md5sum);
		width += MD5SUM_LENGTH;

		memset(identification0, 0, sizeof identification0);
		memcpy(identification0, t->get_rx() + width, sizeof (uint32_t));
		printf("identification0 = 0x%08x, identification = 0x%08x\n", identification0, t->get_identification());
		width += sizeof (uint32_t);

		memset(identification1, 0, sizeof identification1);
		memcpy(identification1, t->get_rx() + width, sizeof (uint32_t));
		printf("identification1 = 0x%08x, identification = 0x%08x\n", identification1, t->get_identification());
		width += sizeof (uint32_t);

//		message = malloc(length + 1);
//		memset(message, 0, sizeof message);
//		memcpy(message, t->get_rx() + width, sizeof length);
//		ret = md5sum(message, length);
		if (ret == -1) {
			return ret;
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
