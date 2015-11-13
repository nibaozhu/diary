.PHONY:
all: test_logging test_regex test_sqlite3 test_list test_base64
clean:
	rm test_logging.o test_logging
	rm test_regex.o test_regex
	rm test_sqlite3.o test_sqlite3
	rm test_list.o test_list

test_base64.o: test_base64.c
test_gnu.o: test_gnu.c
test_logging.o: test_logging.cc ../../include/logging.h
test_regex.o: test_regex.c
test_sqlite3.o: test_sqlite3.c
test_xml.o: test_xml.c
../../include/logging.o: ../../include/logging.cc ../../include/logging.h

test_logging: test_logging.o ../../include/logging.o
	$(CXX) -o $@ $^

test_base64: test_base64.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_regex: test_regex.o
	$(CXX) -o $@ $^

test_sqlite3: test_sqlite3.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_list: test_list.o
	$(CXX) -o $@ $^

install: test_regex test_logging test_sqlite3 test_list
	cp $^ ../../bin

test_list.o: test_list.cc

CPPFLAGS=-I ../../include/
CFLAGS=-g -O0
CXXFLAGS=-g -O0
LDFLAGS=-lsqlite3 -lssl
