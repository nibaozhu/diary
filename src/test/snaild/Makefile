all: snaild

snaild: snaild.o snailp.pb.o
	$(CXX) -o $@ $^ $(LDFLAGS)

snaild.o: snaild.cc snailp.pb.h
snailp.pb.o: snailp.pb.cc snailp.pb.h

snailp.pb.cc snailp.pb.h: snailp.proto
	protoc --cpp_out=./ $^

.PHONY: all clean
clean:
	rm -f snaild snaild.o snailp.pb.* snaild.log

CXXFLAGS=-std=c++11 -Wall -Wno-pointer-arith -O2
LDFLAGS=-pthread -ltcmalloc -lhiredis -llog4cplus -lprotobuf -lz -lzmq
