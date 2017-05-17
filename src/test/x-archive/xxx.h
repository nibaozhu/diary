

struct http_message
{
	int method;
};

struct xxx_cb
{
	int id;
	int (*func_cb)(struct http_message *hm);
};

int f_xxx();

extern struct xxx_cb gc;

#include <stdio.h>

