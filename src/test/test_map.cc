#include <map>
#include <iostream>

int main(void) {
	int ret = 0;

	std::map<int, int> *m = new std::map<int, int>();

	m->insert(std::map<int, int>::value_type(10,0));
	m->insert(std::map<int, int>::value_type(13,3));
	m->insert(std::map<int, int>::value_type(13,4));
	m->insert(std::map<int, int>::value_type(12,2));
	m->insert(std::map<int, int>::value_type(11,1));

	(*m)[99];
	(*m)[98];
	std::map<int, int>::iterator i2 = m->find(15);
	if (i2 != m->end()) {
		std::cout << "found: first = " << i2->first << ", second = " << i2->second << std::endl;
	}

	std::map<int, int>::iterator i = m->begin();

	while (i != m->end()) {
		// std::cout << "erase: first = " << i->first << ", second = " << i->second << std::endl;
		std::cout << ": first = " << i->first << ", second = " << i->second << std::endl;
		// m->erase(i++);
		i++;
	}

	return ret;
}
