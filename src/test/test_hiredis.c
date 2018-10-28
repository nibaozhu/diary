#include <stdio.h>
#include <hiredis/hiredis.h>

int main(int argc, char **argv)
{
	fprintf(stdout, "hiredis %d.%d.%d\n", HIREDIS_MAJOR, HIREDIS_MINOR, HIREDIS_PATCH);

	const char *ip = "127.0.0.1";
	int port = 6379;
	redisContext *context = redisConnect(ip, port);
	if (context == NULL || context->err)
	{
		if (context != NULL)
		{
			fprintf(stderr, "Error: %s\n", context->errstr);
		}
		else
		{
			fprintf(stderr, "Error: redisConnect is NULL\n");
		}

		return 1;
	}

	// char *command = "KEYS *";
	// if (argc > 1)
	// {
	// 	command = argv[1];
	// }

	// redisReply *reply = (redisReply *)redisCommand(context, command);
	redisReply *reply = (redisReply *)redisCommand(context, "HGET %s %s", "snaild", "file://DESKTOP-LPDR5F3/C:/SVN/qhxsl/1-Client/1-Code/qhclient/branch/build-QHRender-Desktop_Qt_5_7_0_MinGW_32bit-Debug/working/0cbbd013-4d76-4266-ab7d-b6ec0af5f5b5/Input/c05-0413.max|2018-10-12 12:34:56|2018-10-12 12:34:56|");
	if (reply == NULL)
	{
		fprintf(stderr, "Error: %s\n", context->errstr);
		return 1;
	}

	size_t i;
	fprintf(stdout, "type: %d, integer: %lld, str: %s\n", reply->type, reply->integer, reply->str);
	for (i = 0; i < reply->elements; i++)
	{
		fprintf(stdout, "str[%d]: %s\n", i, reply->element[i]->str);
	}
	freeReplyObject(reply);
	redisFree(context);
	return 0;
}
