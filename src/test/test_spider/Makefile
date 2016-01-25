CXXFLAGS=-g
CPPFLAGS=-I ../../../include
LDFLAGS=-lcurl


all: s
clean:
	rm s main.o spider.o ../../../include/logging.o

s: main.o spider.o ../../../include/logging.o
	$(CXX) -o $@ $^ $(LDFLAGS)

main.o: main.cpp spider.h ../../../include/logging.h
spider.o: spider.cpp spider.h ../../../include/logging.h
../../../include/logging.o: ../../../include/logging.cc ../../../include/logging.h

.PHONY:all clean
