CPPFLAGS=-I ../../include/
CFLAGS=-g -O0 -std=gnu89
CXXFLAGS=-g -O0 -Wall -Wextra -std=c++0x

PROGRAMS=test_base64 test_client test_digit test_gnu test_or test_routine test_pthread_with_lock test_pthread_with_sync test_regex test_sqlite3 test_xml test_const test_list test_logging test_map test_multimap test_string test_unorderedmap test_curl test_curl test_sem_post test_sem_wait

.PHONY:
all: $(PROGRAMS)
	@ctags * ../../include/*
clean:
	rm *.o
	rm $(PROGRAMS)

test_base64.o: test_base64.c
test_client.o: test_client.c
test_digit.o: test_digit.c
test_gnu.o: test_gnu.c
test_or.o: test_or.c
test_routine.o: test_routine.c
test_pthread_with_lock.o: test_pthread_with_lock.c
test_pthread_with_sync.o: test_pthread_with_sync.c
test_regex.o: test_regex.c
test_sqlite3.o: test_sqlite3.c
test_xml.o: test_xml.c
test_const.o: test_const.cc
test_list.o: test_list.cc
test_logging.o: test_logging.cc ../../include/logging.h
test_map.o: test_map.cc
test_multimap.o: test_multimap.cc
test_string.o: test_string.cc
test_unorderedmap.o: test_unorderedmap.cc
test_curl.o: test_curl.c
test_event.o: test_event.c

test_sem_post.o: test_sem.c
	$(CC) $(CPPFLAGS) -DSEM_POST -c -o $@ $^

test_sem_wait.o: test_sem.c
	$(CC) $(CPPFLAGS) -DSEM_WAIT -c -o $@ $^

../../include/logging.o: ../../include/logging.c ../../include/logging.h


test_logging: test_logging.o ../../include/logging.o
	$(CXX) -o $@ $^ -pthread

test_string: test_string.o
	$(CXX) -o $@ $^

test_map: test_map.o
	$(CXX) -o $@ $^

test_unorderedmap: test_unorderedmap.o
	$(CXX) -o $@ $^

test_multimap: test_multimap.o
	$(CXX) -o $@ $^

test_base64: test_base64.o
	$(CXX) -o $@ $^ -lcrypto
	# $(CXX) -o $@ $^ -lssl

test_const: test_const.o
	$(CXX) -o $@ $^

test_regex: test_regex.o
	$(CXX) -o $@ $^

test_sqlite3: test_sqlite3.o
	$(CXX) -o $@ $^ -lsqlite3

test_list: test_list.o
	$(CXX) -o $@ $^

test_routine: test_routine.o
	$(CXX) -pthread -o $@ $^

test_pthread_with_sync: test_pthread_with_sync.o
	$(CXX) -pthread -o $@ $^

test_pthread_with_lock: test_pthread_with_lock.o
	$(CXX) -pthread -o $@ $^

test_curl: test_curl.o
	$(CC) -lcurl -o $@ $^

test_event: test_event.o
	$(CC) $(pkg-config libevent_pthreads openssl --libs --flags) -o $@ $^

test_sem_post: test_sem_post.o ../../include/logging.o
	$(CC) -lrt -o $@ $^ -pthread
 
test_sem_wait: test_sem_wait.o ../../include/logging.o
	$(CC) -lrt -o $@ $^ -pthread

install: $(PROGRAMS)
	mkdir -p /usr/local/bin/mycode && install $^ /usr/local/bin/mycode
