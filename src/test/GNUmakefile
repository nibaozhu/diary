.PHONY:
all: test_logging test_regex
clean:
	rm test_logging.o test_logging
	rm test_regex.o test_regex

test_logging: test_logging.o
	$(CC) -o $@ $^

test_regex: test_regex.o
	$(CC) -o $@ $^

install: test_regex test_logging
	cp $^ ../../bin

test_logging.o: test_logging.c
test_regex.o: test_regex.c

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
