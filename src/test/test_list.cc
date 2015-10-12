#include <list>
#include <iostream>

int main(void) {
	int ret = 0;

	std::list<int> *r = new std::list<int>();

	r->push_back(22);
	r->push_back(11);
	r->push_back(33);
	std::list<int>::iterator i = r->begin();

	while (i != r->end()) {
		std::cout << "erase " << *i << std::endl;
		i = r->erase(i);
	}

	return ret;
}
