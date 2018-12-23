all: test_lua

test_lua: test_lua.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_lua.o: test_lua.cc

.PHONY: all clean
clean:
	rm --force test_lua test_lua.o

CPPFLAGS=
CXXFLAGS=-Wall
LDFLAGS=/usr/local/lib/liblua.a
