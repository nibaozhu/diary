.PHONY:
all: test_logging test_regex test_sqlite3
clean:
	rm test_logging.o test_logging
	rm test_regex.o test_regex
	rm test_sqlite3.o test_sqlite3

test_logging: test_logging.o
	$(CC) -o $@ $^

test_regex: test_regex.o
	$(CC) -o $@ $^

test_sqlite3: test_sqlite3.o
	$(CC) -o $@ $^

install: test_regex test_logging test_sqlite3
	cp $^ ../../bin

test_logging.o: test_logging.c
test_regex.o: test_regex.c
test_sqlite3.o: test_sqlite3.c

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
