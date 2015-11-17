.PHONY: all clean
all: i1
clean:
	rm -f main.o task.o handle.o ../../include/logging.o i1

i1: main.o task.o handle.o ../../include/logging.o
	$(CXX) -o $@ $^ $(LDFLAGS) && ctags * ../../include/* && svn diff > Changelog

# gcc -I ../../include/ -MM *.cc ../../include/logging.cc -std=gnu++0x
handle.o: handle.cc handle.h transport.h ../../include/logging.h
main.o: main.cc main.h task.h transport.h ../../include/logging.h \
 handle.h
task.o: task.cc task.h transport.h ../../include/logging.h handle.h \
 version.h
../../include/logging.o: ../../include/logging.cc ../../include/logging.h


CPPFLAGS=-I ../../include
CXXFLAGS=-O0 -U NDEBUG -U DEBUG_LOGGING -funsigned-char -g3 -std=c++0x -Wall -Wextra
LDFLAGS=-lssl
