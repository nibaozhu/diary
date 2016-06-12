#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
	const char *path = argv[0];
	const char *mode = "ab";
	FILE *fp = fopen(path, mode);
	if (fp == NULL) {
		fprintf(stdout, "%s(%d)\n", strerror(errno), errno);
	}
	int r = fclose(fp);
	if (r != 0) {
		fprintf(stdout, "%s(%d)\n", strerror(errno), errno);
	}
//	r = fclose(fp);
	if (r != 0) {
		fprintf(stdout, "%s(%d)\n", strerror(errno), errno);
	}
	return 0;
}
