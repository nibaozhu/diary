CPPFLAGS=-U LOGGING_DEBUG
CFLAGS=-fPIC

.PHONY: all
all: liblogging.so liblogging.a

liblogging.so: logging.o
	$(CC) -o $@ $^ -shared

liblogging.a: logging.o
	$(AR) rcs $@ $^

logging.o: logging.c logging.h
