#include <string.h>

#include <array>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
	(void)argc; (void)argv;

	const std::size_t _Nm = 5;
	std::array<std::string, _Nm> arr1;

	char str[] = "123456789";

	for(std::size_t _Im = 0; _Im < arr1.size(); _Im++)
	{
		arr1.at(_Im) = std::string(strfry(str));
	}

	for(std::size_t _Im = 0; _Im < arr1.size(); _Im++)
	{
		std::cout << arr1.at(_Im) << std::endl;
	}

	return 0;
}
