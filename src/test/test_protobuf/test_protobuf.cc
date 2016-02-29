#include "addressbook.pb.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>

int main() {
	int ret = 0;


	GOOGLE_PROTOBUF_VERIFY_VERSION;

	tutorial::AddressBook address_book;
	address_book.set_name("nibaozhu");

	tutorial::PhoneNumber *owner_phone_number = address_book.mutable_phone();
	owner_phone_number->set_number("911");
	owner_phone_number->set_type(tutorial::MOBILE);

	tutorial::Person *person = address_book.add_person();

	person->set_name("John");
	person->set_id(1333);
	person->set_email("john@qq.com");

	tutorial::PhoneNumber *phone_number = person->add_phone();
	phone_number->set_number("+8613903062123");
	phone_number->set_type(tutorial::WORK);

	int s0 = address_book.ByteSize();
	void *m0 = malloc(s0);
	memset(m0, 0, s0);

	std::cout << "address_book: " << address_book.DebugString();
	bool b0 = address_book.SerializeToArray(m0, s0);
	if (b0) {
		std::cout << "ByteSize = " << s0 << std::endl;
	}

	tutorial::AddressBook address_book1;
	address_book1.ParseFromArray(m0, s0);

	int s1 = address_book1.ByteSize();
	void *m1 = malloc(s1);
	memset(m1, 0, s1);

	std::cout << "\n\naddress_book1: " << address_book1.DebugString();
	bool b1 = address_book.SerializeToArray(m1, s1);
	if (b1) {
		std::cout << "ByteSize = " << s1 << std::endl;
	}

	google::protobuf::ShutdownProtobufLibrary();

	return ret;
}

