#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define M1 (1024*1024)
#define M100 (1024*1024*100)
#define M1024 (1024*1024*1024)

int main()
{
	size_t i;
	size_t j;
	uint32_t b[BUFSIZ];
	for(i = 0, j = 0; i < M1024; i++, j++)
	{
		if( j >= BUFSIZ )
		{
			write(1, b, sizeof(uint32_t) * j);
			j = 0;
		}
		b[j] = rand();
	}
#if 0
	write(1, b, sizeof(uint32_t) * j);
#endif
	return 0;
}
