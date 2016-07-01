
#include "enter.h"
#include "common.h"

int main(int argc, char **argv) {
	int value = 3; // TODO: test value
	Enter *e = new Enter(value);

	std::clog << "get_value return = " << e->get_value() << std::endl;
	e->working();

	delete e;
	e = NULL;
	return 0;
}
