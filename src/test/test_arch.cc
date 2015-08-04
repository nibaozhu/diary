#include "something.h"

int main(int argc, char **argv)
{
	int retval = EXIT_SUCCESS;
	do
	{
		retval = initialize_all();
		if (retval == EXIT_FAILURE) break;

		retval = __task(argc, argv);
		if (retval == EXIT_FAILURE) break;

		retval = uninitialize_all();
		if (retval == EXIT_FAILURE) break;
	};
	return retval;
}

int initialize_all()
{
	int retval = EXIT_SUCCESS;

	;;;

	return retval;
}

int uninitialize_all()
{
	int retval = EXIT_SUCCESS;

	;;;

	return retval;
}

int __task()
{
	int retval = EXIT_SUCCESS;
	list<message*> *r;	// read
	list<message*> *w;	// write

	do
	{
		retval = __read(r);
		if (retval == EXIT_FAILURE) break;

		retval = __write(w);
		if (retval == EXIT_FAILURE) break;

		retval = __execute(r, w);
		if (retval == EXIT_FAILURE) break;

	} while (0);
	return retval;
}

int __read
(list<message*> *l)
{
	int retval = EXIT_SUCCESS;
	do
	{
		; //

	} while (0);
	return retval;
}

int __write
(list<message*> *l)
{
	int retval = EXIT_SUCCESS;
	do
	{
		; //

	} while (0);
	return retval;
}

int __execute
(list<message*> *l)
{
	int retval = EXIT_SUCCESS;
	do
	{
		; //

	} while (0);
	return retval;
}





