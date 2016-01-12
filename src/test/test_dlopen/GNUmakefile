.PHONY: all clean
all: main libdynamic.so
clean:
	rm main libdynamic.so main.o dynamic.o

main: main.o
	$(CC) -o $@ $^ -ldl

libdynamic.so: dynamic.o
	$(CC) -o $@ $^ -shared

main.o: main.c
dynamic.o: dynamic.c


LDFLAGS = -ldl
CPPFLAGS = -fPIC
CFLAGS = -g
