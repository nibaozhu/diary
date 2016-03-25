.PHONY: all clean
all: chehou
clean:
	rm -f logging.o handle.o main.o task.o chehou
	rm -rf logdir

chehou: handle.o logging.o main.o task.o
	$(CXX) -o $@ $^ && ctags * && test -d logdir || mkdir logdir

handle.o: handle.cc handle.h transport.h logging.h
logging.o: logging.cc logging.h
main.o: main.cc main.h task.h transport.h logging.h handle.h
task.o: task.cc task.h transport.h logging.h handle.h version.h


CPPFLAGS=-I ./
CXXFLAGS=-O0 -U NDEBUG -D DEBUG_LOGGING -funsigned-char -g3 -std=c++0x -Wall -Wextra -Werror -m64
LDFLAGS=
