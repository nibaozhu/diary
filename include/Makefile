CPPFLAGS=-U LOGGING_DEBUG
CFLAGS=-fPIC

.PHONY: all clean
all: liblogging.so liblogging.a
clean:
	rm liblogging.so liblogging.a logging.o

liblogging.so: logging.o
	$(CC) -o $@ $^ -shared

liblogging.a: logging.o
	$(AR) rcs $@ $^

logging.o: logging.c logging.h
