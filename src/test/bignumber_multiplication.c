#include <stdio.h>
#include <string.h>
#define MAX_LENGTH (100)

int cal(const char *number_x, const char *number_y, char *number_result) {
	char ascii_x[MAX_LENGTH] = {0};
	char ascii_y[MAX_LENGTH] = {0};

	strcpy(ascii_x, number_x);
	strcpy(ascii_y, number_y);

	int length_x = strlen(ascii_x);
	int length_y = strlen(ascii_y);
	int i, j;

	for (i = length_x - 1; i >= 0; i--) {
		ascii_x[i] -= '0';
	}

	for (i = length_y - 1; i >= 0; i--) {
		ascii_y[i] -= '0';
	}

	int carry = 0; // 进位
	int sum = 0;
	for (i = length_x - 1 + length_y - 1; i >= 0; i--) { // i为横坐标
		sum = carry;

		j = i - (length_x - 1);
		if (j < 0) {
			j = 0;
		}

		for (; j <= i && j <= length_y - 1; j++) { // j为纵坐标
			sum += ascii_x[i - j] * ascii_y[j]; // 累计一组数的和
			printf("%d += %d * %d\n", sum, ascii_x[i - j], ascii_y[j]);
		}

		number_result[i + 1] = sum % 10 + '0'; // 保留位
		carry = sum / 10; // 进位
	}

	if (number_result[0] == '0') {
		number_result[0] = ' '; // space
	}

	return 0;
}

int main() {
	char number_x[MAX_LENGTH] = "172";
	char number_y[MAX_LENGTH] = "39";
	char number_result[MAX_LENGTH * 2] = {' '};

	cal(number_x, number_y, number_result);
	puts(number_result);
	return 0;
}
