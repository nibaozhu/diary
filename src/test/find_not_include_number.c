#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define SIZE_500MB (500 * 1024 * 1024)
#define SIZE_500MB (500)

int main() {

	char *buf = malloc ( SIZE_500MB );
	memset(buf, 0, SIZE_500MB);

	unsigned int x = 1;
	int i, j;
	for (i = 0; i < SIZE_500MB; i++) {
		for (j = 0; j < 8; j++) {
			buf[i] = buf[i] | (x<<j);
		}
	}

	buf[0] = 0x5;

	puts("find_not_include_number");
	for (i = 0; i < SIZE_500MB; i++) {
		for (j = 0; j < 8; j++) {
			if ((buf[i] & (x<<j)) != (x<<j)) {
				printf("%d\n", i * 8 + j);
			}
		}
	}

	return 0;
}
