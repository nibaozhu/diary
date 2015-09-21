.PHONY: all clean
all: i1
clean:
	rm -f *.o core.* i1

i1: handle.o main.o task.o
	$(CXX) -o $@ $^ $(LDFLAGS) && ctags *

handle.o: handle.cc handle.h
main.o: main.cc main.h
task.o: task.cc task.h transport.h

CXXFLAGS=-O0 -g -DD -Wno-pointer-arith
LDFLAGS=
