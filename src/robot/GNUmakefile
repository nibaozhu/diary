all: robot

robot: robot.o
	$(CC) -o $@ $^ $(LDFLAGS)

install: robot
	cp robot ../../bin

robot.o: robot.c

.PHONY:
clean:
	rm robot.o robot

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
LDFLAGS=-lsqlite3
