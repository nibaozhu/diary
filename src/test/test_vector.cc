#include <vector>
int main(int argc, char **argv) {

	std::vector<int> v1;

	v1.push_back(5);
	v1.push_back(8);
	v1.push_back(2);
	v1.push_back(1);
	v1.push_back(3);
	v1.pop_back();
	v1.pop_back();
	v1.pop_back();

	int i1 = v1.at(2);

	return 0;
}
