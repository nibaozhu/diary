query: query.o
	$(CC) -o $@ $^

install: query
	cp query ../../bin

query.o: query.c

.PHONY:
clean:
	rm query.o query

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
