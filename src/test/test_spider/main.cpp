#include "spider.h"

extern struct logging *l;

int main(int argc, char **argv) {
	if (argc == 1) {
		return printf("%s [url]\n", argv[0]);
	}

	set_disposition();
	initializing(argv[0], "logdir", "w+", debug, debug, 1, 1, 1024*1024*10);
	spider *s = new spider(argv[1]);
	int ret = s->run();
	delete s;
	uninitialized();
	return ret;
}
