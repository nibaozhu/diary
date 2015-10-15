all: robot

robot: robot.o ../../include/logging.o
	$(CXX) -o $@ $^ $(LDFLAGS)

install: robot
	cp robot ../../bin

robot.o: robot.cc ../../include/logging.h
../../include/logging.o: ../../include/logging.cc ../../include/logging.h

.PHONY:
clean:
	rm robot.o robot ../../include/logging.o

CPPFLAGS=-I ../../include/
CXXFLAGS=-g -O0
LDFLAGS=-lsqlite3
