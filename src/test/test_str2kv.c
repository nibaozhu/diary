#include <stdio.h>
#include <string.h>


struct __kv {
	char kv[10][2][1025];
};

void str2kv(const char *str, struct __kv *kv)
{
	int i = 0;
	int x = 0;
	int y = 0;
	int z = 0;

	for (i = 0; i < strlen(str); i++)
	{
		if (str[i] == '=')
		{
			y = (y == 0)? 1: 0;
			z = 0;
		}
		else if (str[i] == '&')
		{
			x++;
			y = (y == 0)? 1: 0;
			z = 0;
		}
		else
		{
			kv->kv[x][y][z] = str[i];
			z++;
		}
	}

	printf("x=%d, y=%d, z=%d\n", x, y, z);
}

int main(int argc, char **argv)
{
	char url[] = "id=123&name=jack&gender=male&xxxx=ttttttttt";

	struct __kv kv;
	memset(&kv, 0, sizeof kv);

	str2kv(url, &kv);

	return 0;
}
