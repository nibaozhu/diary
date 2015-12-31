#include <stdio.h>
#include <ctype.h>
int main(void)
{
	return printf("%d\n", isdigit('5'));
	// return printf("%d\n", isdigit('9' - '0'));
}
