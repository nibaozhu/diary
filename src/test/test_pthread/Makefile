
CPPFLAGS     = -U DEBUG_LOGGING
CFLAGS       = -O0 -g
LDFLAGS      = -pthread

AR           = ar
RM           = rm


all: a.out

a.out: logging.o a.o reception.o waiter.o
	$(CC) -o $@ $^ $(LDFLAGS)
	$(RM) -rf *.o

.PHONY: all clean

clean:
	$(RM) -rf *.o a.out

###########################################################
# NOTE: DO NOT EDIT, autogen by `gcc -MM *.c'
###########################################################
a.o: a.c a.h common.h logging.h reception.h waiter.h
logging.o: logging.c logging.h
reception.o: reception.c reception.h logging.h
waiter.o: waiter.c waiter.h logging.h
