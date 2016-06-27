#include "enter.h"
#include "common.h"

Enter::Enter(int value): value(value) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return ;
}

Enter::~Enter() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;
	return ;
}

int Enter::get_value() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return this->value;
}

void Enter::set_value(int value) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;
	this->value = value;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return ;
}

void Enter::working() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	do {

		std::clog << "Let me sleeeeeeeeep 1 seconds." << std::endl;
		sleep(1);

		this->receive_from_socket();
		this->send_to_socket();
		this->message_processing();

	} while (true);

	return ;
}

int Enter::receive_from_socket() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	return 0;
}

int Enter::send_to_socket() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	return 0;
}

int Enter::message_processing() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	return 0;
}
