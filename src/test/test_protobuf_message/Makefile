CXXFLAGS=-g

# libprotobuf.so.8 => /usr/local/lib/libprotobuf.so.8 (0x00007f612edb1000)
LDFLAGS=-lprotobuf -L/usr/local/lib/
# LDFLAGS=/usr/local/lib/libprotobuf.a

.PHONY: all clean

all: main

main: main.o hello.pb.o
	$(CXX) -o $@ $^ $(LDFLAGS)

# generate by `gcc -MM *.cc'
main.o: main.cc hello.pb.h
hello.pb.o: hello.pb.cc hello.pb.h


clean:
	rm -rf *.o main
