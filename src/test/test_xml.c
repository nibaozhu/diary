#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv) {
	int ret = 0;
	FILE *fp0 = fopen("f0.xml", "r+");
	FILE *fp1 = fopen("f1.xml", "r+");
	void *pt0 = NULL;
	void *pt1 = NULL;
	int le0 = 0;
	int le1 = 0;
	if (fp0 == NULL || fp1 == NULL) {
		printf("%s(%d)\n", strerror(errno), errno);
		return 1;
	}

	fseek(fp0, 0, SEEK_END);
	fseek(fp1, 0, SEEK_END);
	le0 = ftell(fp0);
	le1 = ftell(fp1);
	fseek(fp0, 0, SEEK_SET);
	fseek(fp1, 0, SEEK_SET);

	pt0 = malloc(le0 + 1);
	pt1 = malloc(le1 + 1);
	memset(pt0, 0, le0 + 1);
	memset(pt1, 0, le1 + 1);

	int size = 1;
	int nmemb = le0;
	ret = fread(pt0, size, nmemb, fp0);
	if (ret < nmemb) {
		return 2;
	}

	nmemb = le1;
	ret = fread(pt1, size, nmemb, fp1);
	if (ret < nmemb) {
		return 3;
	}

	printf("pt0 = {%s}\n", pt0);
	printf("pt1 = {%s}\n", pt1);





	return ret;
}



