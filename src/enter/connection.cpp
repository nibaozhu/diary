
#include "common.h"
#include "connection.h"

Connection::Connection(int value): value(value) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return ;
}

Connection::~Connection() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;
	return ;
}

int Connection::get_value() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return this->value;
}

void Connection::set_value(int value) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;
	this->value = value;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return ;
}

std::string Connection::get_str() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog << "this->str = " << this->str << std::endl;
#endif

	return this->str;
}

void Connection::set_str(std::string str) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;
	this->str = str;

#if DEBUG
	std::clog << "this->str = " << this->str << std::endl;
#endif

	return ;
}

