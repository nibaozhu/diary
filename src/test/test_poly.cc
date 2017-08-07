#include <iostream>


class object
{
public:
	virtual void doFoo();
};

void object::doFoo()
{
	std::cout << "object:: hell" << std::endl;
}

class dog : public object
{
public:
	void doFoo();
};

void dog::doFoo()
{
	std::cout << "dog:: hell" << std::endl;
}

class cat : public object
{
public:
	void doFoo();
};

void cat::doFoo()
{
	std::cout << "cat:: hell" << std::endl;
}


int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	object *ob1 = NULL;

	dog d1;
	ob1 = &d1;
	ob1->doFoo();

	cat c1;
	ob1 = &c1;
	ob1->doFoo();

	object o1;
	ob1 = &o1;
	ob1->doFoo();

	return 0;
}


