CXXFLAGS=-g

myftp: main.o ftplib.o
	$(CXX) -o $@ $^
main.o: main.cpp ftplib.h
ftplib.o: ftplib.cpp ftplib.h

.PHONY:
clean:
	rm myftp *.o
