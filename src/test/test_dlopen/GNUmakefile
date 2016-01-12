.PHONY: all clean
all: libdynamic.so main
	rm dynamic.o main.o
clean:
	rm dynamic.o main.o libdynamic.so main

libdynamic.so: dynamic.o
	$(CC) -o $@ $^ -shared

main: main.o
	$(CC) -o $@ $^ -ldl

dynamic.o: dynamic.c
	$(CC) -c -o $@ $^ -fPIC
main.o: main.c

LDFLAGS = -ldl
CPPFLAGS = 
CFLAGS = -g
