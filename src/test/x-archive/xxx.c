#include "xxx.h"

int echo(struct http_message *hm);
struct xxx_cb gc = { .func_cb = echo };

int echo(struct http_message *hm)
{
	return printf("%s,%d,%s, hm->method = %d\n", __func__, __LINE__, __FILE__, hm->method);
}

int f_xxx()
{
	struct http_message hm = { .method = 9 };
	if (gc.func_cb)
	{
		gc.func_cb(&hm);
	}

	return 0;
}
