
CFLAGS=-g -fPIC
LDFLAGS=-L./ -lxxx

main: main.o libxxx.so
	$(CC) -o $@ $^ $(LDFLAGS) && LD_LIBRARY_PATH=./ PATH=./ $@

libxxx.a: xxx.o
	$(AR) r $@ $^

libxxx.so: xxx.o
	$(CC) -o $@ $^ -shared

.PHONY: clean
clean:
	rm -f *.a *.so *.o main

main.o: main.c xxx.h
xxx.o: xxx.c xxx.h

