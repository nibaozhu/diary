/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::list<Transport*> *c, Transport* t) {
	int ret = 0;
	int i = 0;
	uint32_t length = 0;
	const char *iterator_position = (const char *)t->get_rx();

	t->pr();
	do {
		/* TODO: HTTP PROTOCOL PROXY */

		/* HTTP HEADER */
		const char *haystack = iterator_position;
		const char *needle = (const char *)"\r\n\r\n";
		const char *occurrence_RNRN = strstr(haystack, needle);
		needle = (const char *)"Content-Length:";
		const char *occurrence_CONTENTLENGTH = strstr(haystack, needle);
		uint16_t port = PORT_HTTP;
		LOGGING(debug, "occurrence_RNRN(\\r\\n\\r\\n) = %p\n", occurrence_RNRN);
		LOGGING(debug, "occurrence_CONTENTLENGTH(Content-Length) = %p\n", occurrence_CONTENTLENGTH);
		if (occurrence_RNRN) {

			if (strncmp(haystack, METHOD_GET, strlen(METHOD_GET)) == 0) {
				LOGGING(debug, "%s\n", METHOD_GET);
			} else if (strncmp(haystack, METHOD_POST, strlen(METHOD_POST)) == 0) {
				LOGGING(debug, "%s\n", METHOD_POST);
			} else if (strncmp(haystack, METHOD_CONNECT, strlen(METHOD_CONNECT)) == 0) {
				port = PORT_HTTPS;
				LOGGING(debug, "%s\n", METHOD_CONNECT);
			} else if (strncmp(haystack, METHOD_HTTP, strlen(METHOD_HTTP)) == 0) {

				LOGGING(notice, "%s\n", iterator_position);
				LOGGING(debug, "%s\n", METHOD_HTTP);

				if (occurrence_CONTENTLENGTH) { 
					size_t content_length = atol(occurrence_CONTENTLENGTH + strlen(needle));
					if (content_length + (occurrence_RNRN - haystack) + 4 > t->get_rp()) {
						LOGGING(debug, "Waiting %ld, %ld\n", content_length, t->get_rp());
						break;
					}
				}

				Transport *backt = (*m)[t->get_backfd()];

				std::map<int, Transport*>::iterator im = m->begin();
				im = m->find(t->get_backfd());
				if (im != m->end()) {
				        backt = im->second;
				} else {
					LOGGING(error, "I cannot find backfd (= %d) in m = %p\n", t->get_backfd(), m);
					break;
				}

				backt->set_wx(t->get_rx(), t->get_rp());
				w->push_back(backt);
				t->clear_rx();

				break;
			}

			size_t http_header_length = occurrence_RNRN - haystack + strlen(needle);
			char *http_header = (char *)malloc(http_header_length + 1);
			memset(http_header, 0, http_header_length + 1);
			memcpy(http_header, t->get_rx(), http_header_length);
			LOGGING(info, "--- HTTP HEADER ---\n%s\n------------------------------------------- HTTP ----------\n", http_header);

			haystack = (const char *)http_header;
			needle = (const char *)"Host: ";
			const char *occurrence_Host = strstr(haystack, needle);
			if (occurrence_Host) {

			} else {
				break;
			}

			haystack = occurrence_Host;
			needle = (const char *)"\r\n";
			const char *occurrence_line_ending = strstr(haystack, needle);
			if (occurrence_line_ending) {

			} else {
				break;
			}

			size_t http_header_host_length = occurrence_line_ending - occurrence_Host - strlen("Host: ");
			char *http_header_host = (char *)malloc(http_header_host_length + 1);
			memset(http_header_host, 0, http_header_host_length + 1);
			memcpy(http_header_host, occurrence_Host + strlen("Host: "), http_header_host_length);

			LOGGING(debug, "Host: %s\n", http_header_host);


			struct sockaddr_in peer_addr;
			Transport *__t = new Transport((int)0, (time_t)0, peer_addr, (socklen_t)0, (size_t)SIZE);
			std::string host(http_header_host);
			__t->set_host(host);
			__t->set_port(PORT_HTTP);
			__t->set_backfd(t->get_fd());
			__t->set_wx((void*)iterator_position, http_header_length);

			c->push_back(__t);
#if 0
			ret = connect_to_host(http_header_host, port, iterator_position, http_header_length);
			if (ret == -1) {
				continue;
			}
#endif


			free(http_header_host);
			http_header_host = NULL;

			free(http_header);
			http_header = NULL;

			iterator_position = occurrence_RNRN + strlen("\r\n\r\n");
		} else {
			break;
		}

	} while (true);

	if (t->get_rp() != 0) {
		LOGGING(info, "[!] Transaction(%d) Cancel! length = 0x%x, t->rp = 0x%lx\n", i, length, t->get_rp());
	}
	return ret;
}
