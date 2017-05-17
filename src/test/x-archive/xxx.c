#include "xxx.h"

struct xxx_cb gc;

int f_xxx()
{
	struct http_message hm = { .method = 9 };
	if (gc.func_cb)
	{
		gc.func_cb(&hm);
	}

	return 0;
}
