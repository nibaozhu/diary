.PHONY: all clean
all: xserver
clean:
	$(RM) -f handle.o main.o task.o xserver
	$(RM) -f core.*

xserver: handle.o main.o task.o
	$(CXX) -o $@ $^ $(LDFLAGS)

handle.o: handle.cc handle.h transport.h
main.o: main.cc main.h task.h transport.h handle.h
task.o: task.cc task.h transport.h handle.h version.h

CPPFLAGS=-DNDEBUG
CXXFLAGS=-O0 -g -std=c++0x -Wall -Wextra -Werror
LDFLAGS=-pthread -llogging -lyajl
