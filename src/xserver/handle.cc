/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *__m, Transport* t) {
	int ret = 0;
	int i = 0;

	// NOTE: AppChatProtocol Header
	uint32_t length = 0;
	uint32_t crc32 = 0;
	uint32_t magic = 0;

	t->printread();
	do {
		std::string receiver;

		/* MESSAGE HEADER */
		if (t->get_rp() < 3 * sizeof (uint32_t)) {
			/* pending header message */
			break;
		}

		memcpy(&length, t->get_rx(), sizeof (uint32_t));
		length = ntohl(length);
		if (length > BUFFER_MAX) {
			LOGGING(error, "maybe Handle fail. length = 0x%x(0x%x)\n", length, BUFFER_MAX);
			break;
		}

		memcpy(&crc32, (char *)t->get_rx() + sizeof (uint32_t), sizeof (uint32_t));
		crc32 = ntohl(crc32);

		memcpy(&magic, (char *)t->get_rx() + 2 * sizeof (uint32_t), sizeof (uint32_t));
		magic = ntohl(magic);

		LOGGING(notice, "[+] Transaction(%d) Begin: length = 0x%x, crc32 = 0x%x, magic = 0x%x\n", i, length, crc32, magic);

		/* MESSAGE BODY */
		if (t->get_rp() < 3 * sizeof (uint32_t) + length) {
			/* pending body message */
			break;
		}

		LOGGING(notice, "Message completed(=0x%x).\n", (unsigned int)(3 * sizeof (uint32_t)) + length);

		// TODO: receive array to json
		// ...
		void *body = malloc(length + 1);
		if (!body) {
			LOGGING(error, "malloc fail.\n");
			continue;
		}
		memcpy(body, (char *)t->get_rx() + 3 * sizeof (uint32_t), length);
		char *bodyterminal = (char*)body + length;
		*bodyterminal = '\0';

		const char *input = (char*)body;
		char error_buffer[BUFSIZ];
		size_t error_buffer_size = BUFSIZ;
		yajl_val v = yajl_tree_parse (input, error_buffer, error_buffer_size);
		if (!v) {
			LOGGING(error, "yajl_tree_parse: error_buffer: %s\n", error_buffer);
			continue;
		}

		const char **keys = YAJL_GET_OBJECT(v)->keys;
		yajl_val *values = YAJL_GET_OBJECT(v)->values;
		for (size_t j = 0 ; j < YAJL_GET_OBJECT(v)->len; ++j) {
			LOGGING(info, "j: %u, key: %s, val: %s\n", j, keys[j], YAJL_GET_STRING(values[j]));

			if (strcmp(keys[j], "command") == 0) { t->set_command(YAJL_GET_STRING(values[j])); }
			if (strcmp(keys[j], "context") == 0) {  }
			if (strcmp(keys[j], "dtime") == 0) {  }
			if (strcmp(keys[j], "errcode") == 0) {  }
			if (strcmp(keys[j], "errstring") == 0) {  }
			if (strcmp(keys[j], "passwd") == 0) {  }
			if (strcmp(keys[j], "receiver") == 0) { receiver = YAJL_GET_STRING(values[j]); }
			if (strcmp(keys[j], "sender") == 0) { t->set_appid(YAJL_GET_STRING(values[j])); }
		}

		LOGGING(info, "body: %s\n", (char*)body);
		free(body);

		if (strcmp(t->get_command().c_str(), "LOGON") == 0) {
			std::map<std::string, int>::iterator iter = __m->find(t->get_appid());
			if (iter == __m->end()) {
				__m->insert(std::make_pair(t->get_appid(), t->get_fd()));
				LOGGING(debug, "__m: insert (%s, %d)\n", t->get_appid().c_str(), t->get_fd());
			}
		}

		Transport* tx = t;
		if (strcmp(t->get_command().c_str(), "SEND") == 0) {
			std::map<std::string, int>::iterator iter = __m->find(receiver);
			if (iter != __m->end()) {
				int fx = iter->second;
				std::map<int, Transport*>::iterator im = m->find(fx);
				if (im != m->end()) {
					tx = im->second;
				} else {
					LOGGING(warning, "Connection lost appid: %s\n", receiver.c_str());
					break;
				}
			} else {
				LOGGING(warning, "Session lost appid: %s\n", receiver.c_str());
				break;
			}
		}

		tx->set_wx(t->get_rx(), (3 * sizeof (uint32_t) + length));
		memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (3 * sizeof (uint32_t) + length)), t->get_rp() - (3 * sizeof (uint32_t) + length));
		t->set_rp(t->get_rp() - (3 * sizeof (uint32_t) + length));
		w->push_back(tx);

		LOGGING(info, "[x] Transaction(%d) Passed.\n", ++i);
	} while (true);

	if (t->get_rp() != 0) {
		LOGGING(info, "[!] Transaction(%d) Cancel! length = 0x%x, t->rp = 0x%lx\n", i, length, t->get_rp());
	}
	return ret;
}
