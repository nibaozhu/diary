/* This example code is placed in the public domain. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <gnutls/gnutls.h>

#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
extern int h_errno;

/* A very basic TLS client, with anonymous authentication.
 */

#define MAX_BUF 1024

char url[1024] = "";

int tcp_connect (void);
void tcp_close (int sd);

void usage(const char *argv0) {
  fprintf (stderr, "*** %s [domain] [location]\n", argv0);
  return ;
}

int
main (int argc, char **argv)
{
  if (argc < 3) {
    usage(argv[0]);
    return argc;
  }

  strncpy(url, argv[1], strlen(argv[1]));

  int ret, sd, ii;
  gnutls_session_t session;
  char buffer[MAX_BUF + 1];
  gnutls_anon_client_credentials_t anoncred;
  gnutls_psk_client_credentials_t psk_cred;
  gnutls_certificate_credentials_t xcred;
  /* Need to enable anonymous KX specifically. */

  gnutls_global_init ();

  gnutls_anon_allocate_client_credentials (&anoncred);

  /* Initialize TLS session
   */
  gnutls_init (&session, GNUTLS_CLIENT);

  /* Use default priorities */
  gnutls_priority_set_direct (session, "PERFORMANCE:+ANON-DH:!ARCFOUR-128",
                             NULL);

  gnutls_handshake_set_private_extensions (session, 1);
  gnutls_server_name_set (session, GNUTLS_NAME_DNS, url,
                          strlen (url));

  gnutls_dh_set_prime_bits (session, 512);

  /* put the anonymous credentials to the current session
   */
  gnutls_credentials_set (session, GNUTLS_CRD_ANON, anoncred);

  gnutls_credentials_set (session, GNUTLS_CRD_PSK, psk_cred);
  gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);

  int level = 9;
  gnutls_global_set_log_level (level);

  /* connect to the peer
   */
  sd = tcp_connect ();
  // gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) gl_fd_to_handle(sd));
  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) reinterpret_cast<void*>(sd));

  /* Perform the TLS handshake
   */
  ret = gnutls_handshake (session);

  if (ret < 0)
    {
      fprintf (stderr, "*** Handshake failed(%d)\n", ret);
      gnutls_perror (ret);
      goto end;
    }
  else
    {
      printf ("-- [Handshake was completed]\n");
    }

  char MSG[1024];
  sprintf(MSG, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", argv[2], argv[1]);
  puts(MSG);

  gnutls_record_send (session, MSG, strlen (MSG));

  do {
    ret = gnutls_record_recv (session, buffer, MAX_BUF);
    if (ret == 0)
      {
        printf ("-- [Peer has closed the TLS connection]\n");
        goto end;
      }
    else if (ret < 0)
      {
        fprintf (stderr, "*** Error: %s\n", gnutls_strerror (ret));
        goto end;
      }

    printf ("\n-- [Received %d(%d) bytes]\n", ret, MAX_BUF);
    for (ii = 0; ii < ret; ii++)
      {
        fputc (buffer[ii], stdout);
      }
    fflush(stdout);
  } while (1);
  fputs ("\n", stdout);

  gnutls_bye (session, GNUTLS_SHUT_RDWR);

end:

  tcp_close (sd);

  gnutls_deinit (session);

  gnutls_anon_free_client_credentials (anoncred);

  gnutls_global_deinit ();

  return 0;
}



int tcp_connect (void) {
  int ret = 0;

  int sockfd = 0;
  int domain = AF_INET;
  int type = SOCK_STREAM;
  int protocol = 0;

  sockfd = socket(domain, type, protocol);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = domain;
  addr.sin_port = htons(443);

  struct hostent *ht;
  ht = gethostbyname(url);
  if (ht == NULL)
  {
    herror("gethostbyname");
    return -1;
  }

  char ip_string[4*4] = {0};
  socklen_t size = sizeof ip_string;

  // On success, inet_ntop() returns a non-null pointer to dst.  NULL is returned if there was an error, with errno set to indicate the error.
  inet_ntop(domain, (void *)*(ht->h_addr_list), ip_string, size);
  if (ip_string == NULL) {
    printf("%s\n", strerror(errno));
    return -1;
  }

  ret = inet_pton(domain, ip_string, (struct sockaddr *) &addr.sin_addr.s_addr);
  if (ret != 1) {
    printf("%s\n", strerror(errno));
    return -1;
  }

  ret = connect(sockfd, (struct sockaddr *) &addr, size);
  if (ret == -1) {
    printf("%s\n", strerror(errno));
    return -1;
  }

  return sockfd;
}


void tcp_close (int sd) {
  int ret = 0;
  ret = close(sd);
  if (ret == -1) {
    printf("%s\n", strerror(errno));
    return ;
  }
  return ;
}



