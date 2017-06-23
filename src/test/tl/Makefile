CFLAGS=-g
CXXFLAGS=-g


a.out: logging.o test_logging.o
	$(CXX) -o $@ $^ -pthread

logging.o: logging.c logging.h
test_logging.o: test_logging.cc logging.h
