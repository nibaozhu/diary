.PHONY: a1 clean
a1: main.o enter.o connection.o
	$(CXX) -o $@ $^ && ctags * && rm *.o

main.o: main.cpp common.h enter.h
enter.o: enter.cpp enter.h connection.h common.h
connection.o: connection.cpp connection.h common.h

clean:
	rm -rf main.o enter.o connection.o a1

CXXFLAGS=-D DEBUG -g
