CPPFLAGS=-DNDEBUG
CFLAGS=-fPIC

.PHONY: all clean install
all: liblogging.so liblogging.a
clean:
	$(RM) liblogging.so liblogging.a logging.o

install: all
	install --mode=0644 logging.h /usr/local/include/
	install --mode=0644 liblogging.a /usr/local/lib64/
	install --mode=0755 liblogging.so /usr/local/lib64/

liblogging.so: logging.o
	$(CC) -o $@ $^ -shared

liblogging.a: logging.o
	$(AR) rcs $@ $^

logging.o: logging.c logging.h
