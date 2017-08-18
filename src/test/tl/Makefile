CPPFLAGS=#-D LOGGING_DEBUG
CFLAGS=-g
CXXFLAGS=-g


a.out: ../../../include/logging.o test_logging.o
	$(CXX) -o $@ $^ -pthread

logging.o: ../../../include/logging.c ../../../include/logging.h
test_logging.o: test_logging.cc ../../../include/logging.h
