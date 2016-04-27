all: test_gnutls_client test_gnutls_server test_gnutls_server_X509

test_gnutls_client: test_gnutls_client.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_gnutls_server: test_gnutls_server.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_gnutls_server_X509: test_gnutls_server_X509.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_gnutls_client.o: test_gnutls_client.cc
test_gnutls_server.o: test_gnutls_server.cc
test_gnutls_server_X509.o: test_gnutls_server_X509.cc

.PHONY: all clean
clean:
	rm test_gnutls_client test_gnutls_client.o
	rm test_gnutls_server test_gnutls_server.o
	rm test_gnutls_server_X509 test_gnutls_server_X509.o

CXXFLAGS=-g
LDFLAGS=-lgnutls

