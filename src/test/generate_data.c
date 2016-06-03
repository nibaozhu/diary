#include <stdio.h>
#include <stdint.h>
int main()
{
	long long max = 1024 * 1024 ;
	long long i = 0 ;
#define bufsize 1000
	uint32_t s[bufsize];
	int j = 0 ;
	for ( ; i < max ; i ++ )
	{
		if( j >= bufsize )
		{
			write(1 , s , sizeof(uint32_t) * j);
			j = 0 ;
		}
		s[j] = (uint32_t)random();
		j++;
	}
	write(1 , s , sizeof(uint32_t) * j);
	return 0;
}
