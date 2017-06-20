

CPPFLAGS		= -O0
CFLAGS			= -g
LDFLAGS			= -pthread


a.out: logging.o a.o reception.o waiter.o
	$(CC) -o $@ $^ $(LDFLAGS)
	rm -rf *.o

.PHONY: clean

clean:
	rm -rf *.o a.out

# NOTE: autogen by `gcc -MM *.c'
a.o: a.c a.h common.h logging.h reception.h waiter.h
logging.o: logging.c logging.h
reception.o: reception.c reception.h logging.h
waiter.o: waiter.c waiter.h logging.h
