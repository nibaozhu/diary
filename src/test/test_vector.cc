#include <string.h>

#include <vector>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
	(void)argc; (void)argv;

	// const std::size_t _Nm = 5;
	std::vector<std::string> vec1;

	char str[] = "123456";

	std::cout << "size: " << vec1.size() << ", capacity: "<< vec1.capacity() << std::endl;

	for(std::vector<std::string>::size_type _Im = 0; _Im < 5; _Im++)
	{
		vec1.push_back(std::string(strfry(str)));
	}

	for(std::vector<std::string>::size_type _Im = 0; _Im < vec1.size(); _Im++)
	{
		std::cout << vec1.at(_Im) << std::endl;
	}

	std::cout << "size: " << vec1.size() << ", capacity: "<< vec1.capacity() << std::endl;

	for(std::vector<std::string>::size_type _Im = 0; _Im < 2; _Im++)
	{
		vec1.pop_back();
	}

	std::cout << "size: " << vec1.size() << ", capacity: "<< vec1.capacity() << std::endl;

	// NOTE: compile with -std=c++11
	vec1.shrink_to_fit();

	std::cout << "size: " << vec1.size() << ", capacity: "<< vec1.capacity() << std::endl;

	return 0;
}
