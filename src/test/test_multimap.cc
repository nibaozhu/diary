#include <map>
#include <utility>
#include <iostream>

int main(void) {
	int ret = 0;

	std::multimap<int, int> *m = new std::multimap<int, int>();

	m->insert(std::make_pair(22,2));
	m->insert(std::make_pair(22,2));
	m->insert(std::make_pair(11,1));
	m->insert(std::make_pair(33,3));
	m->insert(std::make_pair(10,0));
	m->insert(std::make_pair(10,0));

	std::multimap<int, int>::iterator i = m->begin();
	while (i != m->end()) {
		std::cout << "erase: first = " << i->first << ", second = " << i->second << std::endl;
		m->erase(i++);
	}

	return ret;
}
