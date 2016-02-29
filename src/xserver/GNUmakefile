.PHONY: all clean
all: xserver
clean:
	rm -f ../../include/logging.o handle.o main.o task.o xserver

xserver: ../../include/logging.o handle.o main.o task.o
	$(CXX) -o $@ $^ && ctags * ../../include/*

# gcc -I ../../include/ -MM *.cc ../../include/logging.cc -std=gnu++0x
handle.o: handle.cc handle.h transport.h ../../include/logging.h
main.o: main.cc main.h task.h transport.h ../../include/logging.h \
 handle.h
task.o: task.cc task.h transport.h ../../include/logging.h handle.h \
 version.h
../../include/logging.o: ../../include/logging.cc ../../include/logging.h


CPPFLAGS=-I ../../include
CXXFLAGS=-O0 -U NDEBUG -U DEBUG_LOGGING -funsigned-char -g3 -std=c++0x -Wall -Wextra -Werror -m64
LDFLAGS=
