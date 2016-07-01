#include "enter.h"
#include "common.h"

Enter::Enter(int value): value(value) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	this->received_message = new std::list<Connection*>();
	this->sending_message = new std::list<Connection*>();
	this->map_connection = new std::map<int, Connection*>();

#if DEBUG
	std::clog
					<< "this = " << this 
					<< ", received_message = " << this->received_message
					<< ", sending_message = " << this->sending_message
					<< ", map_connection = " << this->map_connection
					<< std::endl;

	std::clog << "this->value = " << this->value << std::endl;
#endif

	return ;
}

Enter::~Enter() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog
					<< "this = " << this 
					<< ", received_message = " << this->received_message
					<< ", sending_message = " << this->sending_message
					<< ", map_connection = " << this->map_connection
					<< std::endl;

	int fd = 0;
	Connection *connection = NULL;
	int ret = 0;


	std::map<int, Connection*>::iterator it = this->map_connection->begin();
	for ( ; it != this->map_connection->end(); it++ ) {
		fd = it->first;

		ret = close(fd);
		if (ret == -1) {
			std::clog << strerror(errno) << std::endl;
			continue;	/* XXX: ignore error */
		}

		connection = it->second;
		if (connection) {
			delete connection;
		}
	}

	delete this->received_message;
	delete this->sending_message;
	delete this->map_connection;

	std::clog << "this->value = " << this->value << std::endl;
#endif

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

		if (this->value > 0) {
			std::clog << "this->value = " << this->value << std::endl;
			this->value --;
		} else {
			std::clog << "this->value = " << this->value << " break;" << std::endl;
			break;
		}

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
