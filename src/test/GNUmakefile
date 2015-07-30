test_regex: test_regex.o
	$(CC) -o $@ $^

install: test_regex
	cp test_regex ../../bin

test_regex.o: test_regex.c

.PHONY:
clean:
	rm test_regex.o test_regex

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
