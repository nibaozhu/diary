robot: robot.o
	$(CC) -o $@ $^

install: robot
	cp robot ../../bin

robot.o: robot.c

.PHONY:
clean:
	rm robot.o robot

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
