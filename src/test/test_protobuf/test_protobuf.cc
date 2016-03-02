#include "addressbook.pb.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>

int main() {
	int ret = 0;


	GOOGLE_PROTOBUF_VERIFY_VERSION;

	tutorial::AddressBook ab;
	ab.set_name("nibaozhu");

	tutorial::PhoneNumber *owner_pn = ab.mutable_phone();
	owner_pn->set_number("911");
	owner_pn->set_type(tutorial::MOBILE);

	tutorial::Person *p = ab.add_person();

	p->set_name("John");
	p->set_id(1333);
	p->set_email("john@qq.com");

	tutorial::PhoneNumber *pn = p->add_phone();
	pn->set_number("+8613903062123");
	pn->set_type(tutorial::WORK);

	int s0 = ab.ByteSize();
	void *m0 = malloc(s0);
	memset(m0, 0, s0);

	std::cout << "ab: " << ab.DebugString();
	bool b0 = ab.SerializeToArray(m0, s0);
	if (b0) {
		std::cout << "ByteSize = " << s0 << std::endl;
	}

	tutorial::AddressBook address_book1;
	address_book1.ParseFromArray(m0, s0);

	int s1 = address_book1.ByteSize();
	void *m1 = malloc(s1);
	memset(m1, 0, s1);

	std::cout << "\n\naddress_book1: " << address_book1.DebugString();
	bool b1 = ab.SerializeToArray(m1, s1);
	if (b1) {
		std::cout << "ByteSize = " << s1 << std::endl;
	}

	google::protobuf::ShutdownProtobufLibrary();

	return ret;
}

