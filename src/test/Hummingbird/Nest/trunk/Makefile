all: Nest

Nest: Nest.o Hummingbirdp.pb.o
	$(CXX) -o $@ $^ $(LDFLAGS)

Nest.o: Nest.cc Hummingbirdp.pb.h
Hummingbirdp.pb.o: Hummingbirdp.pb.cc Hummingbirdp.pb.h

Hummingbirdp.pb.cc Hummingbirdp.pb.h: Hummingbirdp.proto
	protoc --cpp_out=./ $^

.PHONY: all clean
clean:
	rm --force Nest Nest.o Hummingbirdp.pb.* /tmp/Nest.log*

CPPFLAGS=-DSNAILD_VERSION_STRING=\"1.0.2\"
CXXFLAGS=-std=c++11 -Wall -Wno-pointer-arith -Wno-deprecated-declarations -Ofast
LDFLAGS=-pthread -ldl -lunwind \
		/usr/local/lib/libtcmalloc.a \
		/usr/local/lib/libhiredis.a \
		/usr/local/lib/liblog4cplus.a \
		/usr/local/lib/libprotobuf-lite.a \
		/usr/local/lib/libz.a \
		/usr/local/lib/libzmq.a

