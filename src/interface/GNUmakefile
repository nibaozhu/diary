.PHONY: all clean
all: i1
clean:
	rm -f *.o core.* i1

i1: handle.o main.o task.o
	$(CXX) -o $@ $^ $(LDFLAGS) && ctags *

handle.o: handle.cc handle.h transport.h
main.o: main.cc main.h task.h transport.h handle.h
task.o: task.cc task.h transport.h handle.h version.h

CXXFLAGS=-O0 -g3 -DD -Wno-pointer-arith -funsigned-char
LDFLAGS=-lssl
