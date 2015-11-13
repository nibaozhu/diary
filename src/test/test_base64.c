#include <openssl/bio.h>
#include <openssl/evp.h>

#include <string.h>

int main(int argc, char **argv) {
		BIO *bio, *b64;
		char message[1024] = "Hello World \n";

		if (argc >= 2) {
			strcpy(message, argv[1]);
		}


		b64 = BIO_new(BIO_f_base64());
		bio = BIO_new_fp(stdout, BIO_NOCLOSE);
		bio = BIO_push(b64, bio);
		BIO_write(bio, message, strlen(message));
		BIO_flush(bio);

		BIO_free_all(bio);
		return 0;
}
