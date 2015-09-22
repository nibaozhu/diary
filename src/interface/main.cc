#include "main.h"

int main(int argc, char **argv) {
	int ret = 0;
	printf("Compile Date/Time: %s %s\n", __DATE__, __TIME__);
	ret = task(argc, argv);
	return ret;
}
