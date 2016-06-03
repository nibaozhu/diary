#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARR_MAX (1<<0x10) // 64KB
#define NMEMB_MAX (1<<0x14) // 1MB

int main() {
	int r = 0;
	FILE *stream = fopen("a.1M", "r");
	if (stream == NULL)
	{
		perror("fopen");
		return errno;
	}

	size_t arr[ARR_MAX] = {0};
	size_t size = sizeof (unsigned int);
	size_t nmemb = NMEMB_MAX;
	size_t rs = 0;
	size_t count = 0;
	size_t i = 0;
	unsigned int *ptr = malloc(nmemb * size);
	memset(ptr, 0, nmemb * size);
	do {
		i = 0;
		rs = fread(ptr, size, nmemb, stream);
		if (rs == -1) {
			perror("fopen");
			return errno;
		}

		while (i < rs) {
			arr[*(ptr + i) >> 0x10]++;
			i++;
		}

		count += rs;
		if (rs < nmemb) {
			printf("last rs = %lx, nmemb = %lx\n", rs, nmemb);
			break;
		}
	} while (1);

	printf("the higher 16bit slot:\n");
	for (i = 0; i < ARR_MAX; i++) {
		if (arr[i]) {
			printf("arr[%lx] = %lx\n", i, arr[i]);
		}
	}

	size_t left_count = 0;
	printf("statistics slot:\n");
	for (i = 0; i < ARR_MAX && left_count <= (count >> 1); i++) {
		left_count += arr[i];
	}

	left_count -= arr[--i]; // backward
	size_t arr_middle = i; // backward
	printf("left_count = %lx\n", left_count);
	printf("left_count = %lx, arr[%lx] = %lx, right_count = %lx, count = %lx\n",
			left_count, i, arr[i], count - left_count - arr[i], count);

	long offset = 0;
	int whence = SEEK_SET;
	r = fseek(stream, offset, whence);
	if (r == -1) {
		perror("fseek");
		return errno;
	}


	memset(ptr, 0, nmemb * size);
	memset(arr, 0, ARR_MAX * sizeof (size_t));
	size_t middle_count = 0;
	do {
		i = 0;
		rs = fread(ptr, size, nmemb, stream);
		if (rs == -1) {
			perror("fopen");
			return errno;
		}

		while (i < rs) {
			if ((*(ptr + i) >> 0x10) == arr_middle) {
				arr[*(ptr + i) & 0xffff]++;
				middle_count++;
			}
			i++;
		}

		if (rs < nmemb) {
			printf("rs = %lx, nmemb = %lx\n", rs, nmemb);
			break;
		}
	} while (1);

	printf("the lower 16bit slot:\n");
	for (i = 0; i < ARR_MAX; i++) {
		if (arr[i]) {
			printf("arr[%lx] = %lx\n", i, arr[i]);
		}
	}

	size_t middle_left_count = 0;
	printf("statistics slot:\n");
	for (i = 0; i < ARR_MAX && left_count + middle_left_count <= (count >> 1); i++) {
		if (arr[i]) {
			middle_left_count += arr[i];
			printf("arr[%lx] = %lx\n", i, arr[i]);
		}
	}

	printf("%lx, %lx, %lx\n", left_count, middle_left_count, count >> 1);

	middle_left_count -= arr[--i]; // backward
	size_t middle_arr_middle = i; // backward
	size_t middle_number = 0;

	size_t previous_slot = 0;
	for (previous_slot = i - 1; previous_slot > 0 && arr[previous_slot] == 0; previous_slot--) { ; }

	if (count & 1) {
		printf("middle_left_count = %lx, arr[%lx] = %lx, middle_right_count = %lx, middle_count = %lx\n",
				middle_left_count, i, arr[i], middle_count - middle_left_count - arr[i], middle_count);

		middle_number = (arr_middle << 0x10) | middle_arr_middle;
	} else {
		printf("middle_left_count = %lx, arr[%lx, %lx] = (%lx, %lx), middle_right_count = %lx, middle_count = %lx\n",
				middle_left_count, previous_slot, middle_arr_middle, arr[previous_slot], arr[middle_arr_middle],
				middle_count - middle_left_count - arr[middle_arr_middle], middle_count);

		middle_number = (arr_middle << 0x10) | ((previous_slot + middle_arr_middle) >> 1);
	}

	printf("middle_number = %lx(hexadecimal), %ld(decemal)\n", middle_number, middle_number);

	free(ptr);
	ptr = NULL;

	r = fclose(stream);
	if (r == EOF) {
		perror("fclose");
		return errno;
	}

	return r;
}
