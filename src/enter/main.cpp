
#include "enter.h"
#include "common.h"

int main(int argc, char **argv) {
	int value = 100; // TODO: test value
	try {
		Enter *e = new Enter(value);
		e->working();
		delete e;
		e = NULL;
	} catch (...) {
		std::cerr << "sorry: encounter exception!" << std::endl;
	}

	return 0;
}
