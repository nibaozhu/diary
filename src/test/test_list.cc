#include <iostream>
#include <cstring>
#include <list>

class C1
{
private:
	char s1[5];
public:
	C1(const char *s1)
	{
		memcpy(this->s1, s1, sizeof s1);
	}

	virtual ~C1()
	{
	}
};

int main(int argc, char **argv)
{
	char s1[5];
	memset(s1, 9, 4);
	C1 *c1;
	std::list<C1*> v1;
	int i = 0;

	while (1)
	{
		if (i > 1024*1024*10)
		{
			std::cerr << __func__ << ": i = " << i << std::endl;
			break;
		}
		i++;

		c1 = new C1(s1);
		if (c1 == NULL)
		{
			std::cerr << __func__ << ": c1 = " << c1 << std::endl;
			break;
		}

		v1.push_back(c1);
	}

	return argc;
}
