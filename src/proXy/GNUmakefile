.PHONY: all clean
all: proXy
clean:
	rm -f ../../include/logging.o handle.o main.o task.o proXy
	rm -f core.*
	rm -rf logdir

proXy: ../../include/logging.o handle.o main.o task.o
	$(CXX) -o $@ $^ && ctags * ../../include/* && test -d logdir || mkdir logdir

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
