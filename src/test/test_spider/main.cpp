#include "spider.h"

int main(int argc, char **argv) {

	if (argc == 1) {
		return printf("%s [url]\n", argv[0]);
	}

	spider s(argv[1]);
	int ret = s.run();
	return ret;
}
