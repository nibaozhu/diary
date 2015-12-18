#include <string>
#include <iostream>

#if 0
void f1(const std::string &s1, std::string &s2)
{
	s2 = s1;
	return ;
}
#endif

int main()
{
	const int i0 = 0;
	int const i1 = 1;
	const int * i2 = &i1;
	int * const i3 = const_cast<int*>(&i1);
	int const * i4 const = const_cast<const int*>(&i1);
	const volatile i5 = 5;

#if 0
	std::string s1("hello");
	std::string s2;
	f1(s1, s2);
	std::cout << "s1 = " << s1 << std::endl;
	std::cout << "s2 = " << s2 << std::endl;
#endif
	return 0;
}
