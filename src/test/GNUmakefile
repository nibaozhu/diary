.PHONY:
all: test_logging test_regex test_sqlite3 test_list
clean:
	rm test_logging.o test_logging
	rm test_regex.o test_regex
	rm test_sqlite3.o test_sqlite3
	rm test_list.o test_list

test_logging: test_logging.o
	$(CC) -o $@ $^

test_regex: test_regex.o
	$(CC) -o $@ $^

test_sqlite3: test_sqlite3.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_list: test_list.o
	$(CXX) -o $@ $^

install: test_regex test_logging test_sqlite3 test_list
	cp $^ ../../bin

test_logging.o: test_logging.c
test_regex.o: test_regex.c
test_sqlite3.o: test_sqlite3.c
test_list.o: test_list.cc

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
CXXFLAGS=-g -O0
LDFLAGS=-lsqlite3
