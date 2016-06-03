
#include "transport.h"

int connect_to_host(const char *host, uint16_t port, const char *buf, size_t count);

#include <sys/select.h>
