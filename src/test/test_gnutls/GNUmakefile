all: test_gnutls_client test_gnutls_server

test_gnutls_client: test_gnutls_client.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_gnutls_server: test_gnutls_server.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_gnutls_client.o: test_gnutls_client.cc
test_gnutls_server.o: test_gnutls_server.cc

.PHONY: all clean
clean:
	rm test_gnutls_client test_gnutls_client.o
	rm test_gnutls_server test_gnutls_server.o

CFLAGS=-g
LDFLAGS=-lgnutls

