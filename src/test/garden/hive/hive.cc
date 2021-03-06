#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <err.h>
#include <arpa/inet.h>
#include <sys/syscall.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent_ssl.h>

#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>

#include <nghttp2/nghttp2.h>
#include <hiredis/hiredis.h>

#include <google/protobuf/util/json_util.h>
#include "hivep.pb.h"

#define OUTPUT_WOULDBLOCK_THRESHOLD (1 << 16)

#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

#define MAKE_NV(NAME, VALUE)                                                   \
  {                                                                            \
    (uint8_t *)NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1,    \
        NGHTTP2_NV_FLAG_NONE                                                   \
  }

int redis_port = 6379;
char redis_host[NAME_MAX] = "127.0.0.1";
char redis_key[NAME_MAX] = "hive";

struct app_context;
typedef struct app_context app_context;

typedef struct http2_stream_data {
  struct http2_stream_data *prev, *next;

  char *method;
  char *path;
  char *authority;
  char *scheme;

  char *user_agent;
  char *content_type;
  char *content_length;
  char *accept_language;
  char *accept;
  char *upgrade_insecure_requests;
  char *accept_encoding;
  char *cache_control;

  const uint8_t *request_context;
  size_t request_context_length;

  int32_t stream_id;
  int fd;

  const uint8_t *respond_context;
  size_t respond_context_length;
  int respond_context_offset;
} http2_stream_data;

typedef struct http2_session_data {
  struct http2_stream_data root;
  struct bufferevent *bev;
  app_context *app_ctx;
  nghttp2_session *session;
  char *client_addr;
} http2_session_data;

struct app_context {
  SSL_CTX *ssl_ctx;
  struct event_base *evbase;
  redisContext *rds_ctx;
};

static unsigned char next_proto_list[256];
static size_t next_proto_list_len;

#ifndef OPENSSL_NO_NEXTPROTONEG
static int next_proto_cb(SSL *ssl, const unsigned char **data,
                         unsigned int *len, void *arg) {
  (void)ssl;
  (void)arg;

  *data = next_proto_list;
  *len = (unsigned int)next_proto_list_len;
  return SSL_TLSEXT_ERR_OK;
}
#endif /* !OPENSSL_NO_NEXTPROTONEG */

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
static int alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
                                unsigned char *outlen, const unsigned char *in,
                                unsigned int inlen, void *arg) {
  int rv;
  (void)ssl;
  (void)arg;

  rv = nghttp2_select_next_protocol((unsigned char **)out, outlen, in, inlen);

  if (rv != 1) {
    return SSL_TLSEXT_ERR_NOACK;
  }

  return SSL_TLSEXT_ERR_OK;
}
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

/* Create SSL_CTX. */
static SSL_CTX *create_ssl_ctx(const char *key_file, const char *cert_file) {
  SSL_CTX *ssl_ctx;
  EC_KEY *ecdh;

  ssl_ctx = SSL_CTX_new(SSLv23_server_method());
  if (!ssl_ctx) {
    errx(1, "Could not create SSL/TLS context: %s",
         ERR_error_string(ERR_get_error(), NULL));
  }
  SSL_CTX_set_options(ssl_ctx,
                      SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                          SSL_OP_NO_COMPRESSION |
                          SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

  ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
  if (!ecdh) {
    errx(1, "EC_KEY_new_by_curv_name failed: %s",
         ERR_error_string(ERR_get_error(), NULL));
  }
  SSL_CTX_set_tmp_ecdh(ssl_ctx, ecdh);
  EC_KEY_free(ecdh);

  if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM) != 1) {
    errx(1, "Could not read private key file %s", key_file);
  }
  if (SSL_CTX_use_certificate_chain_file(ssl_ctx, cert_file) != 1) {
    errx(1, "Could not read certificate file %s", cert_file);
  }

  next_proto_list[0] = NGHTTP2_PROTO_VERSION_ID_LEN;
  memcpy(&next_proto_list[1], NGHTTP2_PROTO_VERSION_ID,
         NGHTTP2_PROTO_VERSION_ID_LEN);
  next_proto_list_len = 1 + NGHTTP2_PROTO_VERSION_ID_LEN;

#ifndef OPENSSL_NO_NEXTPROTONEG
  SSL_CTX_set_next_protos_advertised_cb(ssl_ctx, next_proto_cb, NULL);
#endif /* !OPENSSL_NO_NEXTPROTONEG */

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
  SSL_CTX_set_alpn_select_cb(ssl_ctx, alpn_select_proto_cb, NULL);
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

  return ssl_ctx;
}

/* Create SSL object */
static SSL *create_ssl(SSL_CTX *ssl_ctx) {
  SSL *ssl;
  ssl = SSL_new(ssl_ctx);
  if (!ssl) {
    errx(1, "Could not create SSL/TLS session object: %s",
         ERR_error_string(ERR_get_error(), NULL));
  }
  return ssl;
}

static void add_stream(http2_session_data *session_data,
                       http2_stream_data *stream_data) {
  stream_data->next = session_data->root.next;
  session_data->root.next = stream_data;
  stream_data->prev = &session_data->root;
  if (stream_data->next) {
    stream_data->next->prev = stream_data;
  }
}

static void remove_stream(http2_session_data *session_data,
                          http2_stream_data *stream_data) {
  (void)session_data;

  stream_data->prev->next = stream_data->next;
  if (stream_data->next) {
    stream_data->next->prev = stream_data->prev;
  }
}

static http2_stream_data *
create_http2_stream_data(http2_session_data *session_data, int32_t stream_id) {
  http2_stream_data *stream_data = (http2_stream_data *)malloc(sizeof(http2_stream_data));
  memset(stream_data, 0, sizeof(http2_stream_data));
  stream_data->stream_id = stream_id;
  stream_data->fd = -1;

  add_stream(session_data, stream_data);
  return stream_data;
}

static void delete_http2_stream_data(http2_stream_data *stream_data) {
  if (stream_data->fd != -1) {
    close(stream_data->fd);
  }
  free(stream_data->method);
  free(stream_data->path);
  free(stream_data->authority);
  free(stream_data->scheme);

  free(stream_data->user_agent);
  free(stream_data);
}

static http2_session_data *create_http2_session_data(app_context *app_ctx,
                                                     int fd,
                                                     struct sockaddr *addr,
                                                     int addrlen) {
  int rv;
  http2_session_data *session_data = (http2_session_data *)malloc(sizeof(http2_session_data));
  SSL *ssl;
  char host[NI_MAXHOST];
  int val = 1;

  ssl = create_ssl(app_ctx->ssl_ctx);
  memset(session_data, 0, sizeof(http2_session_data));
  session_data->app_ctx = app_ctx;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(val));
  session_data->bev = bufferevent_openssl_socket_new(
      app_ctx->evbase, fd, ssl, BUFFEREVENT_SSL_ACCEPTING,
      BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
  bufferevent_enable(session_data->bev, EV_READ | EV_WRITE);
  rv = getnameinfo(addr, (socklen_t)addrlen, host, sizeof(host), NULL, 0,
                   NI_NUMERICHOST);
  if (rv != 0) {
    session_data->client_addr = strdup("(unknown)");
  } else {
    session_data->client_addr = strdup(host);
  }

  return session_data;
}

static void delete_http2_session_data(http2_session_data *session_data) {
  http2_stream_data *stream_data;
  SSL *ssl = bufferevent_openssl_get_ssl(session_data->bev);
  fprintf(stderr, "%s disconnected\n", session_data->client_addr);
  if (ssl) {
    SSL_shutdown(ssl);
  }
  bufferevent_free(session_data->bev);
  nghttp2_session_del(session_data->session);
  for (stream_data = session_data->root.next; stream_data;) {
    http2_stream_data *next = stream_data->next;
    delete_http2_stream_data(stream_data);
    stream_data = next;
  }
  free(session_data->client_addr);
  free(session_data);
}

/* Serialize the frame and send (or buffer) the data to
   bufferevent. */
static int session_send(http2_session_data *session_data) {
  int rv;
  rv = nghttp2_session_send(session_data->session);
  if (rv != 0) {
    warnx("Fatal error: %s", nghttp2_strerror(rv));
    return -1;
  }
  return 0;
}

/* Read the data in the bufferevent and feed them into nghttp2 library
   function. Invocation of nghttp2_session_mem_recv() may make
   additional pending frames, so call session_send() at the end of the
   function. */
static int session_recv(http2_session_data *session_data) {
  ssize_t readlen;
  struct evbuffer *input = bufferevent_get_input(session_data->bev);
  size_t datalen = evbuffer_get_length(input);
  unsigned char *data = evbuffer_pullup(input, -1);

  readlen = nghttp2_session_mem_recv(session_data->session, data, datalen);
  if (readlen < 0) {
    warnx("Fatal error: %s", nghttp2_strerror((int)readlen));
    return -1;
  }
  if (evbuffer_drain(input, (size_t)readlen) != 0) {
    warnx("Fatal error: evbuffer_drain failed");
    return -1;
  }
  if (session_send(session_data) != 0) {
    return -1;
  }
  return 0;
}

static ssize_t send_callback(nghttp2_session *session, const uint8_t *data,
                             size_t length, int flags, void *user_data) {
  http2_session_data *session_data = (http2_session_data *)user_data;
  struct bufferevent *bev = session_data->bev;
  (void)session;
  (void)flags;

  /* Avoid excessive buffering in server side. */
  if (evbuffer_get_length(bufferevent_get_output(session_data->bev)) >=
      OUTPUT_WOULDBLOCK_THRESHOLD) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }
  bufferevent_write(bev, data, length);
  return (ssize_t)length;
}

/* Returns nonzero if the string |s| ends with the substring |sub| */
static int ends_with(const char *s, const char *sub) {
  size_t slen = strlen(s);
  size_t sublen = strlen(sub);
  if (slen < sublen) {
    return 0;
  }
  return memcmp(s + slen - sublen, sub, sublen) == 0;
}

/* Returns int value of hex string character |c| */
static uint8_t hex_to_uint(uint8_t c) {
  if ('0' <= c && c <= '9') {
    return (uint8_t)(c - '0');
  }
  if ('A' <= c && c <= 'F') {
    return (uint8_t)(c - 'A' + 10);
  }
  if ('a' <= c && c <= 'f') {
    return (uint8_t)(c - 'a' + 10);
  }
  return 0;
}

/* Decodes percent-encoded byte string |value| with length |valuelen|
   and returns the decoded byte string in allocated buffer. The return
   value is NULL terminated. The caller must free the returned
   string. */
static char *percent_decode(const uint8_t *value, size_t valuelen) {
  char *res;

  res = (char *)malloc(valuelen + 1);
  if (valuelen > 3) {
    size_t i, j;
    for (i = 0, j = 0; i < valuelen - 2;) {
      if (value[i] != '%' || !isxdigit(value[i + 1]) ||
          !isxdigit(value[i + 2])) {
        res[j++] = (char)value[i++];
        continue;
      }
      res[j++] =
          (char)((hex_to_uint(value[i + 1]) << 4) + hex_to_uint(value[i + 2]));
      i += 3;
    }
    memcpy(&res[j], &value[i], 2);
    res[j + 2] = '\0';
  } else {
    memcpy(res, value, valuelen);
    res[valuelen] = '\0';
  }
  return res;
}

static ssize_t file_read_callback(nghttp2_session *session, int32_t stream_id,
                                  uint8_t *buf, size_t length,
                                  uint32_t *data_flags,
                                  nghttp2_data_source *source,
                                  void *user_data) {
  int fd = source->fd;
  ssize_t r;
  (void)user_data;
  http2_stream_data *stream_data = (http2_stream_data *)nghttp2_session_get_stream_user_data(session, stream_id);
  if (!stream_data) {
    return 0;
  }

  fprintf(stderr, "fd: %d\n", fd);
  if (fd > 0 && stream_data->respond_context_length == 0) {
    while ((r = read(fd, buf, length)) == -1 && errno == EINTR)
      ;
  } else {
    if (stream_data->respond_context_offset + length < stream_data->respond_context_length) {
      r = length;
      fprintf(stderr, "datlen: %lu(>%lu) too long, chunked\n", stream_data->respond_context_length, length);
    } else {
      r = stream_data->respond_context_length - stream_data->respond_context_offset;
    }
    stream_data->respond_context_offset = r;
    memcpy(buf, stream_data->respond_context, r);
  }

  if (r == -1) {
    return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
  }
  if ((size_t)r < length) {
    *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    stream_data->respond_context_offset = 0;
  }
  return r;
}

static int send_response(nghttp2_session *session, int32_t stream_id,
                         nghttp2_nv *nva, size_t nvlen, int fd) {
  int rv;
  nghttp2_data_provider data_prd;
  data_prd.source.fd = fd;
  data_prd.read_callback = file_read_callback;

  rv = nghttp2_submit_response(session, stream_id, nva, nvlen, &data_prd);
  if (rv != 0) {
    warnx("Fatal error: %s", nghttp2_strerror(rv));
    return -1;
  }
  return 0;
}

static const char ERROR_HTML[] = "<html><head><title>404</title></head>"
                                 "<body><h1>404 Not Found</h1></body></html>";

static int error_reply(nghttp2_session *session,
                       http2_stream_data *stream_data) {
  int rv;
  ssize_t writelen;
  int pipefd[2];
  nghttp2_nv hdrs[] = {MAKE_NV(":status", "404")};

  rv = pipe(pipefd);
  if (rv != 0) {
    warn("Could not create pipe");
    rv = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                   stream_data->stream_id,
                                   NGHTTP2_INTERNAL_ERROR);
    if (rv != 0) {
      warnx("Fatal error: %s", nghttp2_strerror(rv));
      return -1;
    }
    return 0;
  }

  writelen = write(pipefd[1], ERROR_HTML, sizeof(ERROR_HTML) - 1);
  close(pipefd[1]);

  if (writelen != sizeof(ERROR_HTML) - 1) {
    close(pipefd[0]);
    return -1;
  }

  stream_data->fd = pipefd[0];

  if (send_response(session, stream_data->stream_id, hdrs, ARRLEN(hdrs),
                    pipefd[0]) != 0) {
    close(pipefd[0]);
    return -1;
  }
  return 0;
}

/* nghttp2_on_header_callback: Called when nghttp2 library emits
   single header name/value pair. */
static int on_header_callback(nghttp2_session *session,
                              const nghttp2_frame *frame, const uint8_t *name,
                              size_t namelen, const uint8_t *value,
                              size_t valuelen, uint8_t flags, void *user_data) {
  http2_stream_data *stream_data;

  const char METHOD[] = ":method";
  const char PATH[] = ":path";
  const char AUTHORITY[] = ":authority";
  const char SCHEME[] = ":scheme";

  const char USER_AGENT[] = "user-agent";
  const char CONTENT_TYPE[] = "content-type";
  const char CONTENT_LENGTH[] = "content-length";
  const char ACCEPT_LANGUAGE[] = "accept-language";
  const char ACCEPT[] = "accept";
  const char UPGRADE_INSECURE_REQUESTS[] = "upgrade-insecure-requests";
  const char ACCEPT_ENCODING[] = "accept-encoding";
  const char CACHE_CONTROL[] = "cache-control";

  (void)flags;
  (void)user_data;

  fprintf(stdout, "%s: %s\n", name, value);
  switch (frame->hd.type) {
  case NGHTTP2_HEADERS:
    if (frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
      break;
    }
    stream_data =
        (http2_stream_data *)nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
    if (!stream_data) {
      break;
    }
    if (namelen == sizeof(METHOD) - 1 && memcmp(METHOD, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->method = percent_decode(value, j);
    }
    if (namelen == sizeof(PATH) - 1 && memcmp(PATH, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->path = percent_decode(value, j);
    }
    if (namelen == sizeof(AUTHORITY) - 1 && memcmp(AUTHORITY, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->authority = percent_decode(value, j);
    }
    if (namelen == sizeof(SCHEME) - 1 && memcmp(SCHEME, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->scheme = percent_decode(value, j);
    }
    if (namelen == sizeof(USER_AGENT) - 1 && memcmp(USER_AGENT, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->user_agent = percent_decode(value, j);
    }
    if (namelen == sizeof(CONTENT_TYPE) - 1 && memcmp(CONTENT_TYPE, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->content_type = percent_decode(value, j);
    }
    if (namelen == sizeof(CONTENT_LENGTH) - 1 && memcmp(CONTENT_LENGTH, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->content_length = percent_decode(value, j);
    }
    break;
  }
  return 0;
}

static int on_begin_headers_callback(nghttp2_session *session,
                                     const nghttp2_frame *frame,
                                     void *user_data) {
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;

  if (frame->hd.type != NGHTTP2_HEADERS ||
      frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
    return 0;
  }
  stream_data = create_http2_stream_data(session_data, frame->hd.stream_id);
  nghttp2_session_set_stream_user_data(session, frame->hd.stream_id,
                                       stream_data);
  return 0;
}

/* Minimum check for directory traversal. Returns nonzero if it is
   safe. */
static int check_path(const char *path) {
  /* We don't like '\' in url. */
  return path[0] && path[0] == '/' && strchr(path, '\\') == NULL &&
         strstr(path, "/../") == NULL && strstr(path, "/./") == NULL &&
         !ends_with(path, "/..") && !ends_with(path, "/.");
}

static int hivep_signup(redisContext *rds_ctx, std::string &telephone, std::string &passwd) {
  int r = 0;

  fprintf(stdout, "#SIGNUP# telephone: '%s', passwd: '%s'\n", telephone.c_str(), passwd.c_str());
  redisReply *reply = (redisReply *)redisCommand(rds_ctx, "%s %s %s", "SETNX", telephone.c_str(), passwd.c_str());
  if (reply == NULL) {
    r = redisReconnect(rds_ctx);
    fprintf(stderr, "redisReconnect: '%s', r: %d\n", rds_ctx->errstr, r);
    return -1;
  }

  if (reply->type == REDIS_REPLY_INTEGER && reply->integer >= 0) {
    if (reply->integer == 1) {
      fprintf(stdout, "signup success\n");
      r = 1;
    }
    fprintf(stdout, "type: %d integer: %lld, str: '%s'\n", reply->type, reply->integer, reply->str);
  } else {
    fprintf(stderr, "type: %d integer: %lld, str: '%s'\n", reply->type, reply->integer, reply->str);
    r = -1;
  }
  freeReplyObject(reply);
  return r;
}

static int handle_hivep(http2_session_data *session_data,
                hivep::Request &request, hivep::Respond &respond) {
  int r = 0;
  request.PrintDebugString();

  if (request.has_signin()) {
    request.signin().hivenumber();
//  request.signin().email();
    request.signin().qrcode();
    std::string passwd = request.signin().passwd();
    std::string clientversion = request.signin().clientversion();
    std::string osversion = request.signin().osversion();
    std::string osmanufacturer = request.signin().osmanufacturer();
  } else if (request.has_signup()) {
//  request.signup().email();
    std::string telephone = request.signup().telephone();
    std::string passwd = request.signup().passwd();
    std::string nickname = request.signup().nickname();

    r = hivep_signup(session_data->app_ctx->rds_ctx, telephone, passwd);
    if (r == 1) {
      // respond.set_errnum((int64_t)errnum_);
      // respond.set_errstring(errstring_);
    }

  } else if (request.has_getfriends()) {
    std::string hiveid = request.getfriends().hiveid();
  } else if (request.has_deletefriend()) {
    std::string hiveid = request.deletefriend().hiveid();
    uint64_t hivenumber = request.deletefriend().hivenumber();
  } else if (request.has_sendmessage()) {
    std::string hiveid = request.sendmessage().hiveid();
    uint64_t otherhivenumber = request.sendmessage().otherhivenumber();
  } else if (request.has_signout()) {
    std::string hiveid = request.signout().hiveid();
  } else if (request.has_heartbeat()) {
    std::string hiveid = request.heartbeat().hiveid();
    std::string extra = request.heartbeat().extra();
  } else {
    fprintf(stderr, "not found invalid operation\n");
  }


  return r;
}

static int handle_http2(nghttp2_session *session,
                           http2_session_data *session_data,
                           http2_stream_data *stream_data)
{
  int r = 0;
  if (!stream_data->content_type) {
    stream_data->content_type = (char *)malloc(sizeof (char));
    memset(stream_data->content_type, 0, sizeof (char));
  }

  size_t n = strlen(stream_data->content_type);
  hivep::Request request;
  hivep::Respond respond;

  if (memcmp(stream_data->content_type, "application/x-www-form-urlencoded",
                          n) == 0) {
    // TODO: parse key1=value1&key2=value2&.., and decode url.
    //
    //

    // stream_data->respond_context = stream_data->request_context;
    // stream_data->respond_context_length = stream_data->request_context_length;
    // stream_data->respond_context_offset = 0;

  } else if (memcmp(stream_data->content_type, "application/json",
                          n) == 0) {
    std::string input((const char *)stream_data->request_context, stream_data->request_context_length);
    google::protobuf::util::Status s = google::protobuf::util::JsonStringToMessage(input, 
                    (google::protobuf::Message *)&request);
    if (!s.ok()) {
      fprintf(stderr, "JsonStringToMessage fails, %s!\n", s.error_message().as_string().c_str());
    } else {
      respond.set_seq(request.seq());
    }
  } else if (memcmp(stream_data->content_type, "application/x-protobuf",
                          n) == 0) {
    if (!request.ParseFromArray(stream_data->request_context, stream_data->request_context_length)) {
      fprintf(stderr, "ParseFromArray fails!\n");
    } else {
      respond.set_seq(request.seq());
    }
  } else {
    fprintf(stderr, "invalid Content-Type: '%s'\n", stream_data->content_type);

    stream_data->respond_context = stream_data->request_context;
    stream_data->respond_context_length = stream_data->request_context_length;
    stream_data->respond_context_offset = 0;
  }

  pid_t tid = syscall(SYS_gettid);
  respond.set_tid(tid);
 
  struct tm tm_;
  struct timeval tv_;
  gettimeofday(&tv_, NULL);
  localtime_r(&tv_.tv_sec, &tm_);
  int64_t currentMSecsSinceEpoch = (int64_t)tv_.tv_sec * 1000 + tv_.tv_usec / 1000;
  respond.set_created(currentMSecsSinceEpoch);

  handle_hivep(session_data, request, respond);

#if 0
  stream_data->respond_context_length = respond.ByteSizeLong();
  stream_data->respond_context = (const uint8_t *)malloc(stream_data->respond_context_length);
  if (!respond.SerializeToArray((void *)stream_data->respond_context, stream_data->respond_context_length))
  {
    free((void *)stream_data->respond_context); // XXX: reset to zero
    return r; // XXX: return value ...
  }
#endif

  std::string output;
  google::protobuf::util::Status s1 = google::protobuf::util::MessageToJsonString(respond, &output);
  stream_data->respond_context_length = output.length();
  stream_data->respond_context = (const uint8_t *)malloc(stream_data->respond_context_length + 1);
  stream_data->respond_context_offset = 0;
  memset((void *)stream_data->respond_context, 0, stream_data->respond_context_length + 1);
  memcpy((void *)stream_data->respond_context, (const void *)output.c_str(), stream_data->respond_context_length);
  fprintf(stderr, "output: '%s'\n", output.c_str());
  fprintf(stderr, "respond_context: '%s'(%ld)\n", stream_data->respond_context, stream_data->respond_context_length);

  return r;
}

static int on_request_recv(nghttp2_session *session,
                           http2_session_data *session_data,
                           http2_stream_data *stream_data) {
  int fd;
  nghttp2_nv hdrs[] = {MAKE_NV(":status", "200")};
  char *rel_path;

  if (!stream_data->path) {
    if (error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }
  fprintf(stdout, "From %s\n", session_data->client_addr);
  // fwrite((char *)stream_data->request_context, 1, stream_data->request_context_length, stdout);
  // fprintf(stdout, "\n");

  /**************************
   * `hivep.proto'
   * ************************/
  handle_http2(session, session_data, stream_data);

  if (!check_path(stream_data->path)) {
    if (error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }
  for (rel_path = stream_data->path; *rel_path == '/'; ++rel_path)
    ;
  fd = open(rel_path, O_RDONLY);
  if (fd == -1) {
    if (error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }
  stream_data->fd = fd;

  if (send_response(session, stream_data->stream_id, hdrs, ARRLEN(hdrs), fd) !=
      0) {
    close(fd);
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }
  return 0;
}

static int on_frame_recv_callback(nghttp2_session *session,
                                  const nghttp2_frame *frame, void *user_data) {
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;
  switch (frame->hd.type) {
  case NGHTTP2_DATA:
    /* Check that the client request stream has finished */
    if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
      stream_data =
          (http2_stream_data *)nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
      /* For DATA frame,  */
      if (!stream_data) {
        return 0;
      }
      return on_request_recv(session, session_data, stream_data);
    }
    break;
  case NGHTTP2_HEADERS:
    /* Check that the client request stream has finished */
    if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
      stream_data =
          (http2_stream_data *)nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
      /* For HEADERS frame,  */
      if (!stream_data) {
        return 0;
      }
      return on_request_recv(session, session_data, stream_data);
    }
    break;
  case NGHTTP2_SETTINGS:
  case NGHTTP2_PING:
  case NGHTTP2_WINDOW_UPDATE:
    /* Check that the client request headers has finished */
    if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
      stream_data =
          (http2_stream_data *)nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
      /* For HEADERS frame,  */
      if (!stream_data) {
        return 0;
      }
      return on_request_recv(session, session_data, stream_data);
    }
    break;
  default:
    break;
  }
  return 0;
}

static int on_data_chunk_recv_callback(nghttp2_session *session, uint8_t flags,
                                       int32_t stream_id, const uint8_t *data,
                                       size_t len, void *user_data) {
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;
  (void)session;
  (void)flags;

  stream_data = (http2_stream_data *)nghttp2_session_get_stream_user_data(session, stream_id);
  if (!stream_data) {
    return 0;
  }

  stream_data->request_context = (const uint8_t *)realloc((void *)stream_data->request_context, stream_data->request_context_length + len);
  if (stream_data->request_context) {
    memcpy((void *)stream_data->request_context + stream_data->request_context_length, data, len);
    stream_data->request_context_length += len;
  }
  return 0;
}

static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                    uint32_t error_code, void *user_data) {
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;
  (void)error_code;

  stream_data = (http2_stream_data *)nghttp2_session_get_stream_user_data(session, stream_id);
  if (!stream_data) {
    return 0;
  }
  remove_stream(session_data, stream_data);
  delete_http2_stream_data(stream_data);
  return 0;
}

static void initialize_nghttp2_session(http2_session_data *session_data) {
  nghttp2_session_callbacks *callbacks;

  nghttp2_session_callbacks_new(&callbacks);

  nghttp2_session_callbacks_set_send_callback(callbacks, send_callback);

  nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks,
                                                       on_frame_recv_callback);

  nghttp2_session_callbacks_set_on_data_chunk_recv_callback(
    callbacks, on_data_chunk_recv_callback);

  nghttp2_session_callbacks_set_on_stream_close_callback(
      callbacks, on_stream_close_callback);

  nghttp2_session_callbacks_set_on_header_callback(callbacks,
                                                   on_header_callback);

  nghttp2_session_callbacks_set_on_begin_headers_callback(
      callbacks, on_begin_headers_callback);

  nghttp2_session_server_new(&session_data->session, callbacks, session_data);

  nghttp2_session_callbacks_del(callbacks);
}

/* Send HTTP/2 client connection header, which includes 24 bytes
   magic octets and SETTINGS frame */
static int send_server_connection_header(http2_session_data *session_data) {
  nghttp2_settings_entry iv[1] = {
      {NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100}};
  int rv;

  rv = nghttp2_submit_settings(session_data->session, NGHTTP2_FLAG_NONE, iv,
                               ARRLEN(iv));
  if (rv != 0) {
    warnx("Fatal error: %s", nghttp2_strerror(rv));
    return -1;
  }
  return 0;
}

/* readcb for bufferevent after client connection header was
   checked. */
static void readcb(struct bufferevent *bev, void *ptr) {
  http2_session_data *session_data = (http2_session_data *)ptr;
  (void)bev;

  if (session_recv(session_data) != 0) {
    delete_http2_session_data(session_data);
    return;
  }
}

/* writecb for bufferevent. To greaceful shutdown after sending or
   receiving GOAWAY, we check the some conditions on the nghttp2
   library and output buffer of bufferevent. If it indicates we have
   no business to this session, tear down the connection. If the
   connection is not going to shutdown, we call session_send() to
   process pending data in the output buffer. This is necessary
   because we have a threshold on the buffer size to avoid too much
   buffering. See send_callback(). */
static void writecb(struct bufferevent *bev, void *ptr) {
  http2_session_data *session_data = (http2_session_data *)ptr;
  if (evbuffer_get_length(bufferevent_get_output(bev)) > 0) {
    return;
  }
  if (nghttp2_session_want_read(session_data->session) == 0 &&
      nghttp2_session_want_write(session_data->session) == 0) {
    delete_http2_session_data(session_data);
    return;
  }
  if (session_send(session_data) != 0) {
    delete_http2_session_data(session_data);
    return;
  }
}

/* eventcb for bufferevent */
static void eventcb(struct bufferevent *bev, short events, void *ptr) {
  http2_session_data *session_data = (http2_session_data *)ptr;
  if (events & BEV_EVENT_CONNECTED) {
    const unsigned char *alpn = NULL;
    unsigned int alpnlen = 0;
    SSL *ssl;
    (void)bev;

    fprintf(stderr, "%s connected\n", session_data->client_addr);

    ssl = bufferevent_openssl_get_ssl(session_data->bev);

#ifndef OPENSSL_NO_NEXTPROTONEG
    SSL_get0_next_proto_negotiated(ssl, &alpn, &alpnlen);
#endif /* !OPENSSL_NO_NEXTPROTONEG */
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
    if (alpn == NULL) {
      SSL_get0_alpn_selected(ssl, &alpn, &alpnlen);
    }
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

    if (alpn == NULL || alpnlen != 2 || memcmp("h2", alpn, 2) != 0) {
      fprintf(stderr, "%s h2 is not negotiated\n", session_data->client_addr);
      delete_http2_session_data(session_data);
      return;
    }

    initialize_nghttp2_session(session_data);

    if (send_server_connection_header(session_data) != 0 ||
        session_send(session_data) != 0) {
      delete_http2_session_data(session_data);
      return;
    }

    return;
  }
  if (events & BEV_EVENT_EOF) {
    fprintf(stderr, "%s EOF\n", session_data->client_addr);
  } else if (events & BEV_EVENT_ERROR) {
    fprintf(stderr, "%s network error\n", session_data->client_addr);
  } else if (events & BEV_EVENT_TIMEOUT) {
    fprintf(stderr, "%s timeout\n", session_data->client_addr);
  }
  delete_http2_session_data(session_data);
}

/* callback for evconnlistener */
static void listenercb(struct evconnlistener *listener, evutil_socket_t fd,
                     struct sockaddr *addr, int addrlen, void *user_arg) {
  app_context *app_ctx = (app_context *)user_arg;
  http2_session_data *session_data;
  (void)listener;

  session_data = create_http2_session_data(app_ctx, fd, addr, addrlen);

  bufferevent_setcb(session_data->bev, readcb, writecb, eventcb, session_data);
}

static void start_listen(struct event_base *evbase, const char *service,
                         app_context *app_ctx) {
  int rv;
  struct addrinfo hints;
  struct addrinfo *res, *rp;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
#ifdef AI_ADDRCONFIG
  hints.ai_flags |= AI_ADDRCONFIG;
#endif /* AI_ADDRCONFIG */

  rv = getaddrinfo(NULL, service, &hints, &res);
  if (rv != 0) {
    errx(1, "Could not resolve server address");
  }
  for (rp = res; rp; rp = rp->ai_next) {
    struct evconnlistener *listener;
    int backlog = (1<<10);
    listener = evconnlistener_new_bind(
        evbase, listenercb, app_ctx, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        backlog, rp->ai_addr, (int)rp->ai_addrlen);
    if (listener) {
      freeaddrinfo(res);

      return;
    }
  }
  errx(1, "Could not start listener");
}

static void initialize_app_context(app_context *app_ctx, SSL_CTX *ssl_ctx,
                                   struct event_base *evbase, redisContext *rds_ctx) {
  memset(app_ctx, 0, sizeof(app_context));
  app_ctx->ssl_ctx = ssl_ctx;
  app_ctx->evbase = evbase;
  app_ctx->rds_ctx = rds_ctx;
}

static void run(const char *service, const char *key_file,
                const char *cert_file) {
  SSL_CTX *ssl_ctx;
  app_context app_ctx;
  struct event_base *evbase;
  redisContext *rds_ctx;

  ssl_ctx = create_ssl_ctx(key_file, cert_file);
  evbase = event_base_new();
  rds_ctx = redisConnect(redis_host, redis_port);
  if (rds_ctx == NULL || rds_ctx->err) {
    if (rds_ctx != NULL) {
      fprintf(stderr, "redisConnect: %s\n", rds_ctx->errstr);
      return ;
    }
  }

  initialize_app_context(&app_ctx, ssl_ctx, evbase, rds_ctx);
  start_listen(evbase, service, &app_ctx);

  event_base_loop(evbase, 0);

  event_base_free(evbase);
  SSL_CTX_free(ssl_ctx);
}

int main(int argc, char **argv) {

  struct sigaction act;

  if (argc < 4) {
    fprintf(stderr, "Usage: %s PORT KEY_FILE CERT_FILE\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, NULL);

  SSL_load_error_strings();
  SSL_library_init();

  run(argv[1], argv[2], argv[3]);

  return 0;
}

