#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>


size_t my_write( void *ptr, size_t size, size_t nmemb, void *stream) {
	fwrite(ptr, size, nmemb, stream);
	printf("ptr = %p,\tsize = %lx,\tnmemb = %lx,\tstream = %p\n", ptr, size, nmemb, stream);
	return size * nmemb;
}

int i_want_to_get_this_content(const char *url) {
	CURL *handle;
	CURLcode errornum;
	CURLoption option = CURLOPT_URL;
	const char * parameter = url;

	handle = curl_easy_init( );
	if (handle == NULL) {
		return 1;
	}

	errornum = curl_easy_setopt(handle, option, parameter);
	if (errornum != CURLE_OK) {
		puts(curl_easy_strerror(errornum));
		return errornum;
	}

	option = CURLOPT_ENCODING;
	errornum = curl_easy_setopt(handle, option, "identity,deflate,gzip");
	if (errornum != CURLE_OK) {
		puts(curl_easy_strerror(errornum));
		return errornum;
	}

	option = CURLOPT_WRITEFUNCTION;
	errornum = curl_easy_setopt(handle, option, my_write);
	if (errornum != CURLE_OK) {
		puts(curl_easy_strerror(errornum));
		return errornum;
	}

	errornum = curl_easy_perform(handle);
	if (errornum != CURLE_OK) {
		puts(curl_easy_strerror(errornum));
		return errornum;
	}

	curl_easy_cleanup(handle);
	return 0;
}

int main(int ac, char **av) {
	return i_want_to_get_this_content(av[1]);
}

