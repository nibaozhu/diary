#include "xxx.h"

extern struct xxx_cb gc;

int hello(struct http_message *hm)
{
	return printf("%s,%d,%s, hm->method = %d\n", __func__, __LINE__, __FILE__, hm->method);
}

int main(int argc, char **argv)
{
	int x = 9;
	printf("%p, %p\n", &gc.func_cb, gc.func_cb);
	f_xxx();

	gc.func_cb = hello;
	printf("%p, %p\n", &gc.func_cb, gc.func_cb);
	f_xxx();

	return 0;
}
