test_gnutls: test_gnutls.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_gnutls.o: test_gnutls.cc

.PHONY:
clean:
	rm test_gnutls test_gnutls.o

CFLAGS=-g
LDFLAGS=-lgnutls

