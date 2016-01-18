.PHONY:all clean
all: s
clean:
	rm s main.o spider.o

s: main.o spider.o
	$(CXX) -o $@ $^

main.o: main.cpp spider.h

spider.o: spider.cpp spider.h

CXXFLAGS=-g
