/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "main.h"

int main(int argc, char **argv) {
	int ret = 0;
	printf("Release: %s %s\n", __DATE__, __TIME__);
	ret = task(argc, argv);
	return ret;
}
