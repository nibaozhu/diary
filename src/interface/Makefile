.PHONY: all clean
all: i
clean:
	rm -f *.o *.core i

i: handle.o main.o task.o
	$(CXX) -o $@ $^ $(LDFLAGS)

main.o: main.cc main.h
task.o: task.cc task.h transport.h
handle.o: handle.cc handle.h

CXXFLAGS=-O0 -g
LDFLAGS=-D1
