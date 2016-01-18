#include <curl/curl.h>

int main(void) {


	CURL *handle;

	CURLcode errornum;
	CURLoption option = CURLOPT_URL;
	char * parameter = "http://www.sohu.com";

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
	parameter = "identity,deflate,gzip";
	errornum = curl_easy_setopt(handle, option, parameter);
	if (errornum != CURLE_OK) {
		puts(curl_easy_strerror(errornum));
		return errornum;
	}

#if 0
	errornum = curl_easy_setopt(handle, option, parameter);
	if (errornum != CURLE_OK) {
		puts(curl_easy_strerror(errornum));
		return errornum;
	}
#endif

	errornum = curl_easy_perform(handle);
	if (errornum != CURLE_OK) {
		puts(curl_easy_strerror(errornum));
		return errornum;
	}

	curl_easy_cleanup(handle);

	return 0;
}
