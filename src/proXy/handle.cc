/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"


int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *__m, Transport* t) {
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
		uint16_t port = PORT_HTTP;
		plog(debug, "occurrence_RNRN(\\r\\n\\r\\n) = %p\n", occurrence_RNRN);
		if (occurrence_RNRN) {

			if (strncmp(haystack, METHOD_GET, strlen(METHOD_GET)) == 0) {
				plog(debug, "%s\n", METHOD_GET);
			} else if (strncmp(haystack, METHOD_POST, strlen(METHOD_POST)) == 0) {
				plog(debug, "%s\n", METHOD_POST);
			} else if (strncmp(haystack, METHOD_CONNECT, strlen(METHOD_CONNECT)) == 0) {
				port = PORT_HTTPS;
				plog(debug, "%s\n", METHOD_CONNECT);
			}

			size_t http_header_length = occurrence_RNRN - haystack + strlen(needle);
			char *http_header = (char *)malloc(http_header_length + 1);
			memset(http_header, 0, http_header_length + 1);
			memcpy(http_header, t->get_rx(), http_header_length);
			plog(info, "--- HTTP HEADER ---\n%s\n------------------------------------------- HTTP ----------\n", http_header);

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

			plog(debug, "Host: %s\n", http_header_host);

			ret = connect_to_host(http_header_host, port, iterator_position, http_header_length);
			if (ret == -1) {
				continue;
			}


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
		plog(info, "[!] Transaction(%d) Cancel! length = 0x%x, t->rp = 0x%lx\n", i, length, t->get_rp());
	}
	return ret;
}
