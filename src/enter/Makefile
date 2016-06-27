.PHONY: a1 clean
a1: main.o enter.o
	$(CXX) -o $@ $^ && ctags *

main.o: main.cpp common.h enter.h
enter.o: enter.cpp enter.h

clean:
	rm -rf main.o enter.o a1
