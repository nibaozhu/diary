#include <string.h>
#include <openssl/des.h>

void printF(const_DES_cblock *s) {
	int i = 0;
	for ( ; i < 8; i++) { printf("%02X ", (*s)[i]); }
	printf("\n");
}

int main(int argc, char **argv) {

	// const char *des3_key = "012345670123456701234567";
	const char *des3_key = "ZGFyayBtYXR0ZXIgZXNjYXBl";
	const char *cleartext = "abcde";

	if (argc > 1) { cleartext = argv[1]; }

	const_DES_cblock key1;
	const_DES_cblock key2;
	const_DES_cblock key3;

	DES_key_schedule ks1;
	DES_key_schedule ks2;
	DES_key_schedule ks3;

	memcpy(key1, des3_key, 8);
	memcpy(key2, des3_key+8, 8);
	memcpy(key3, des3_key+16, 8);

	int ret = DES_set_key(&key1, &ks1);
	ret = DES_set_key(&key2, &ks2);
	ret = DES_set_key(&key3, &ks3);

	const_DES_cblock input;
	DES_cblock output;

	int enc = DES_ENCRYPT;

	int i = 0, j = 0;
	for ( ; i < strlen(cleartext); i += 8) {
		memset(input, 0, 8);
		if (i + 8 > strlen(cleartext)) {
			memcpy(input, cleartext + i, strlen(cleartext) - i);
		} else {
			memcpy(input, cleartext + i, 8);
		}
		printf("S %d\t", j);
		printF(&input);
		DES_ecb3_encrypt(&input, &output, &ks1, &ks2, &ks3, enc);
		printf("D %d\t", j++);
		printF(&output);
	}

	return 0;
}
