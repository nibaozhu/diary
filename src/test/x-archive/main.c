#include "xxx.h"

extern struct xxx_cb gc;

int hello(struct http_message *hm)
{
	return printf("%s,%d,%s, hm->method = %d\n", __func__, __LINE__, __FILE__, hm->method);
}

int main(int argc, char **argv)
{
	int x = 9;
	gc.func_cb = hello;

	return f_xxx();
}
