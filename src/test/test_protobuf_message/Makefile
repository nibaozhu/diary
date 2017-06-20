CPPFLAGS=-D PROTOBUF4C_MAIN
CXXFLAGS=-g

# LDFLAGS=-lprotobuf -L/usr/local/lib/
LDFLAGS=/usr/local/lib/libprotobuf.a
# LDFLAGS=`pkg-config --libs protobuf`

.PHONY: all clean

all: main

main: main.o hello.pb.o
	$(CXX) -o $@ $^ $(LDFLAGS)

# generate by `gcc -MM *.cc'
main.o: main.cc hello.pb.h
hello.pb.o: hello.pb.cc hello.pb.h


clean:
	rm -rf *.o main
