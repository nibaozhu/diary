all: nest

nest: nest.o hummingbirdp.pb.o
	$(CXX) -o $@ $^ $(LDFLAGS)

nest.o: nest.cc hummingbirdp.pb.h
hummingbirdp.pb.o: hummingbirdp.pb.cc hummingbirdp.pb.h

hummingbirdp.pb.cc hummingbirdp.pb.h: hummingbirdp.proto
	protoc --cpp_out=./ $^

.PHONY: all clean
clean:
	rm --force nest nest.o hummingbirdp.pb.* /tmp/nest.log*

CPPFLAGS=-DNEST_VERSION_STRING=\"1.4.0\"
CXXFLAGS=-std=c++11 -Wall -Wno-pointer-arith -Wno-deprecated-declarations -Ofast
LDFLAGS=-pthread -ldl -lunwind \
		/usr/local/lib/libtcmalloc.a \
		/usr/local/lib/libhiredis.a \
		/usr/local/lib/liblog4cplus.a \
		/usr/local/lib/libprotobuf-lite.a \
		/usr/local/lib/libz.a \
		/usr/local/lib/libzmq.a

