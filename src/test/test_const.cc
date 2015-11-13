#include <string>
#include <iostream>

void f1(const std::string &s1, std::string &s2)
{
	s2 = s1;
	return ;
}

int main()
{
	std::string s1("hello");
	std::string s2;
	f1(s1, s2);
	std::cout << "s1 = " << s1 << std::endl;
	std::cout << "s2 = " << s2 << std::endl;
}
