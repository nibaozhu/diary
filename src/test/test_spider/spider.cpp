

#include "spider.h"


std::string content;

size_t my_write(void *ptr, size_t size, size_t nmemb, void *stream) {
	// fwrite(ptr, size, nmemb, stream);
	printf("ptr = %p,\tsize = %lx,\tnmemb = %lx,\tstream = %p\n", ptr, size, nmemb, stream);

	content.append((char *)ptr, size * nmemb);
	return size * nmemb;
}

