
#include "hello.pb.h"
#include <iostream>
#include <string>


int Print(::google::protobuf::Message* message) {

	std::cout << "====================" << std::endl;
	std::cout << "DebugString: " << std::endl;
	std::cout << message->DebugString() << std::endl;
	std::cout << "____________________" << std::endl;

	std::cout << "ShortDebugString: " << std::endl;
	std::cout << message->ShortDebugString() << std::endl;
	std::cout << "____________________" << std::endl;

	std::cout << "Utf8DebugString: " << std::endl;
	std::cout << message->Utf8DebugString() << std::endl;
	std::cout << "____________________" << std::endl;

	std::cout << "PrintDebugString: " << std::endl;
	message->PrintDebugString();
	std::cout << "====================" << std::endl;

	return 0;
}


int main(int argc, char **argv) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	// hello::HeartbeatRequest heartbeatRequest;

	/* generate by `uuidgen' */
	// std::string sequence("192a8d4a-1be8-4014-bd8a-9dddf14645f6");
	// heartbeatRequest.set_sequence(sequence);

	// Print(&heartbeatRequest);


	/////////////////////////////////////////////////////////////////////////
	// REQUEST MAKER
	hello::DefaultRequest defaultRequest;

	std::string sequence = "95f901b4-6d6f-4661-aa39-077cdd0c12f2";
	defaultRequest.set_sequence(sequence);

	int32_t type = rand();
	defaultRequest.set_type(type);

	defaultRequest.set_column3(rand());
	defaultRequest.set_column4(rand());
	defaultRequest.set_column5(rand());
	defaultRequest.set_column6(rand());
	defaultRequest.set_column7(rand());

	Print(&defaultRequest);

	std::string serializedString = "";
	std::cout << ":[" << serializedString << "]" << std::endl;

	serializedString = defaultRequest.SerializeAsString();
	std::cout << "SerializeAsString:[" << serializedString << "]" << std::endl;

	defaultRequest.SerializeToString(&serializedString);
	std::cout << "SerializeToString:[" << serializedString << "]" << std::endl;




	/////////////////////////////////////////////////////////////////////////
	// REQUEST RECEIVER
	hello::DefaultRequest defaultRequest2;
	std::string serializedString2 = serializedString;

	defaultRequest2.ParseFromString(serializedString2);

	Print(&defaultRequest2);


	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}



