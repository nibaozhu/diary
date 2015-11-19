/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "main.h"

int main(int argc, char **argv) {
	int ret = 0;
	plog(info, "%s(%d)\n", __func__, ret = task(argc, argv));
	return ret;
}
