/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"

yajl_gen_status appchat_yajl_gen_string(yajl_gen hand, const char * str, size_t len) {
	return yajl_gen_string(hand, (const unsigned char*)str, len);
}

int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<std::string, int> *__m, Transport* t) {
	int i = 0;

	// NOTE: AppChatProtocol Header
	uint32_t length = 0;
	uint32_t crc32 = 0;
	uint32_t magic = 0;

	t->printread();
	do {
		/* MESSAGE HEADER */
		if (t->get_rp() < 3 * sizeof (uint32_t)) {
			/* pending header message */
			break;
		}

		memcpy(&length, t->get_rx(), sizeof (uint32_t));
		length = ntohl(length);
		if (length > BUFFER_MAX) {
			LOGGING(error, "maybe Handle fail. length = 0x%x(0x%x)\n", length, BUFFER_MAX);
			t->clear_rx();
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
			t->clear_rx();
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
			t->clear_rx();
			continue;
		}

		const char **keys = YAJL_GET_OBJECT(v)->keys;
		yajl_val *values = YAJL_GET_OBJECT(v)->values;
		for (size_t j = 0 ; j < YAJL_GET_OBJECT(v)->len; ++j) {
			LOGGING(info, "j: %u, key: %s, val: %s\n", j, keys[j], YAJL_GET_STRING(values[j]));

			if (strcmp(keys[j], "command") == 0) t->command = YAJL_GET_STRING(values[j]);
			if (strcmp(keys[j], "context") == 0) t->context = YAJL_GET_STRING(values[j]);
			if (strcmp(keys[j], "dtime") == 0) t->dtime = YAJL_GET_STRING(values[j]);
			if (strcmp(keys[j], "errcode") == 0) t->errcode = YAJL_GET_STRING(values[j]);
			if (strcmp(keys[j], "errstring") == 0) t->errstring = YAJL_GET_STRING(values[j]);
			if (strcmp(keys[j], "passwd") == 0) t->passwd = YAJL_GET_STRING(values[j]);
			if (strcmp(keys[j], "receiver") == 0) t->receiver = YAJL_GET_STRING(values[j]);
			if (strcmp(keys[j], "sender") == 0) t->sender = YAJL_GET_STRING(values[j]);
		}

		LOGGING(info, "body: %s\n", (char*)body);
		free(body);

		yajl_gen hand = yajl_gen_alloc(NULL);
		yajl_gen_map_open(hand);

		if (strcmp(t->command.c_str(), "LOGON") == 0) {
			t->appid = t->sender;
			std::map<std::string, int>::iterator iter = __m->find(t->appid);
			if (iter == __m->end()) {
				__m->insert(std::make_pair(t->appid, t->get_fd()));
				LOGGING(debug, "__m: insert (%s, %d)\n", t->appid.c_str(), t->get_fd());
			}

			appchat_yajl_gen_string(hand, "command", strlen("command"));
			appchat_yajl_gen_string(hand, "LOGON-RESPONSE", strlen("LOGON-RESPONSE"));

			appchat_yajl_gen_string(hand, "dtime", strlen("dtime"));
			appchat_yajl_gen_string(hand, t->dtime.c_str(), t->dtime.length());

			appchat_yajl_gen_string(hand, "errcode", strlen("errcode"));
			appchat_yajl_gen_string(hand, t->errcode.c_str(), t->errcode.length());

			appchat_yajl_gen_string(hand, "errstring", strlen("errstring"));
			appchat_yajl_gen_string(hand, t->errstring.c_str(), t->errstring.length());

			appchat_yajl_gen_string(hand, "relationship", strlen("relationship"));
			yajl_gen_array_open(hand);
			for (iter = __m->begin(); iter != __m->end(); iter++) {
				LOGGING(debug, "__m: (%s: %d)\n", iter->first.c_str(), iter->second);

				yajl_gen_map_open(hand);
				appchat_yajl_gen_string(hand, "appid", strlen("appid"));
				appchat_yajl_gen_string(hand, iter->first.c_str(), iter->first.length());

				appchat_yajl_gen_string(hand, "context", strlen("context"));
				appchat_yajl_gen_string(hand, "...", strlen("..."));
				yajl_gen_map_close(hand);
			}
			yajl_gen_array_close(hand);
		}

		Transport* tx = t;
		if (strcmp(t->command.c_str(), "SEND") == 0) {
			std::map<std::string, int>::iterator iter = __m->find(t->receiver);
			if (iter != __m->end()) {
				int fx = iter->second;
				std::map<int, Transport*>::iterator im = m->find(fx);
				if (im != m->end()) {
					tx = im->second;
				} else {
					LOGGING(warning, "Connection lost appid: %s\n", t->receiver.c_str());
					break;
				}
			} else {
				LOGGING(warning, "Session lost appid: %s\n", t->receiver.c_str());
				break;
			}

			appchat_yajl_gen_string(hand, "dtime", strlen("dtime"));
			appchat_yajl_gen_string(hand, t->dtime.c_str(), t->dtime.length());

			appchat_yajl_gen_string(hand, "sender", strlen("sender"));
			appchat_yajl_gen_string(hand, t->sender.c_str(), t->sender.length());

			appchat_yajl_gen_string(hand, "receiver", strlen("receiver"));
			appchat_yajl_gen_string(hand, t->receiver.c_str(), t->receiver.length());

			appchat_yajl_gen_string(hand, "context", strlen("context"));
			appchat_yajl_gen_string(hand, t->context.c_str(), t->context.length());

			appchat_yajl_gen_string(hand, "command", strlen("command"));
			appchat_yajl_gen_string(hand, "RECEIVE", strlen("RECEIVE"));
		}

		yajl_gen_map_close(hand);
		const unsigned char *buf = NULL;
		size_t len = 0;
		yajl_gen_get_buf(hand, &buf, &len);
		LOGGING(info, "push(%lu): %s\n", len, buf);

		uint32_t push_length = 0;
		uint32_t push_crc32 = 0;
		uint32_t push_magic = 0;

		push_length = htonl(len);
		tx->set_wx((void*)&push_length, sizeof (uint32_t));

		// FIXME: calc `len' crc
		push_crc32 = htonl(len);
		tx->set_wx((void*)&push_crc32, sizeof (uint32_t));

		push_magic = htonl(magic);
		tx->set_wx((void*)&push_magic, sizeof (uint32_t));

		tx->set_wx((void*)buf, len);
		w->push_back(tx);

		memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (3 * sizeof (uint32_t) + length)), t->get_rp() - (3 * sizeof (uint32_t) + length));
		t->set_rp(t->get_rp() - (3 * sizeof (uint32_t) + length));

		LOGGING(info, "[x] Transaction(%d) Passed.\n", ++i);
	} while (true);

	if (t->get_rp() != 0) {
		LOGGING(warning, "[!] Transaction(%d) Cancel! length = 0x%x, t->rp = 0x%lx\n", i, length, t->get_rp());
	}
	return i;
}
