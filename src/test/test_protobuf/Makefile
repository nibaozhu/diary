test_protobuf: test_protobuf.o addressbook.pb.o
	g++ -o test_protobuf test_protobuf.o addressbook.pb.o -pthread -L/usr/local/lib -lprotobuf

addressbook.pb.o: addressbook.pb.cc addressbook.pb.h
	g++ -c -g -o addressbook.pb.o addressbook.pb.cc
test_protobuf.o: test_protobuf.cc addressbook.pb.h
	g++ -c -g -o test_protobuf.o test_protobuf.cc

.PHONY: pb clean
pb:
	protoc --cpp_out=./ addressbook.proto
clean:
	rm *.pb.* test_protobuf.o test_protobuf
