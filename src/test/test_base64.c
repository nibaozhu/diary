#include <openssl/bio.h>
#include <openssl/evp.h>

int main() {
		BIO *bio, *b64;
		char message[] = "Hello World \n";

		b64 = BIO_new(BIO_f_base64());
		bio = BIO_new_fp(stdout, BIO_NOCLOSE);
		bio = BIO_push(b64, bio);
		BIO_write(bio, message, strlen(message));
		BIO_flush(bio);

		BIO_free_all(bio);
		return 0;
}
