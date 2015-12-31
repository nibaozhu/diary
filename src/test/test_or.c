#include <stdio.h>
#include <stdlib.h>
int main(int ac, char **argv) {
	// unsigned char c1 = 0x63;
	unsigned char c1 = atoi(&argv[1][2]);
	unsigned int i1 = (c1 >> 4) ^ (c1 & 0x0f);
	return printf("i1 = %u\n", i1);
}
