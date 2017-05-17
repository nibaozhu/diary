
CFLAGS=-g

main: main.o libxxx.a
	cc -o $@ $^ && ${PWD}/$@

libxxx.a: xxx.o
	ar  r $@ $^

.PHONY: clean
clean:
	rm -f *.a *.o main

main.o: main.c xxx.h
xxx.o: xxx.c xxx.h

