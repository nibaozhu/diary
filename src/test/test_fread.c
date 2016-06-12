#include <stdio.h>
#include <stdlib.h>

int main() {
	const char *path = "a.1";
	const char *mode = "r";

	FILE *stream = fopen(path, mode);
	if (!stream) { return 1; }

	void *ptr = malloc(10);
	size_t size = 1;
	size_t nmemb = 2;
	size_t sret = fread(ptr, size, nmemb, stream);

	return 0;
}
