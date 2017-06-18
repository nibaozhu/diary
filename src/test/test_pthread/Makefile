

CPPFLAGS		= -O0
CFLAGS			= -g
LDFLAGS			= -pthread


a.out: a.o worker.o
	$(CC) -o $@ $^ $(LDFLAGS)
	rm -rf *.o

worker.o: worker.c a.h worker.h
a.o: a.c a.h worker.h

.PHONY: clean

clean:
	rm -rf *.o a.out
