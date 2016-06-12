#include <stdio.h>

int main(int argc, char **argv)
{
	FILE *fp = NULL;
	const char *path = argv[1];
	const char *mode = "ab";
	fp = fopen(path, mode);
	if (fp == NULL) {
		perror("");
		return 0;
	}

	long position = ftell(fp);
	if (position == -1) {
		perror("");
		return 0;
	}
	
	printf("position = %ld\n", position);
	return 0;
}
