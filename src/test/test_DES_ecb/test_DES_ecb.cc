#include <string.h>
#include <openssl/des.h>

void printF(const_DES_cblock *s) {
	int i = 0;
	for ( ; i < 8; i++) { printf("%02X ", (*s)[i]); }
	printf("\n");
}

int main(int argc, char **argv) {

	const char *des3_key = "012345670123456701234567";
	const char *cleartext = "01234567";

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

	memcpy(input, cleartext, 8);

	int enc = DES_ENCRYPT;

	printF(&input);
	DES_ecb_encrypt(&input, &output, &ks1, enc);
	printF(&output);

	return 0;
}
