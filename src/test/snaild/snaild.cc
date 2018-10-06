#include <arpa/inet.h>
#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <paths.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <gperftools/tcmalloc.h>
#include <hiredis/hiredis.h>
#include <zlib.h>
#include <zmq.h>

#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/version.h>

#include "snailp.pb.h"

#ifndef __set_errno
# define __set_errno(Val) errno = (Val)
#endif

log4cplus::Logger root = log4cplus::Logger::getRoot();
void *zmq_ctx;
void *dealer;
void *router;

rlim_t threads = 1;
char workspace[PATH_MAX] = _PATH_TMP;

void my_free(void *data, void *hint)
{
	tc_free(data);
}

void *hiredis_routine(void *arg);
void *snaild_routine(void *arg);
void received_message(void *socket, void *receivedBuffer, int receivedBufferSize);
bool fragment_to_file(const snailp::TransferRequest &transferRequest, snailp::TransferRespond &transferRespond);
int snaild_term();

int main(int argc, char **argv)
{
	int zmq_major;
	int zmq_minor;
	int zmq_patch;
	zmq_version (&zmq_major, &zmq_minor, &zmq_patch);

	char banner[NAME_MAX];
	snprintf(banner, NAME_MAX, "%s version (%s %s), %s, hiredis %d.%d.%d, log4cplus %s, protobuf %s, zlib %s, zmq %d.%d.%d\n",
			argv[0], __DATE__, __TIME__,
			TC_VERSION_STRING, HIREDIS_MAJOR, HIREDIS_MINOR, HIREDIS_PATCH,
			LOG4CPLUS_VERSION_STR, google::protobuf::internal::VersionString(GOOGLE_PROTOBUF_VERSION).c_str(),
			zlib_version, zmq_major, zmq_minor, zmq_patch);
	bool flags = false;
	int opt;
	while ((opt = getopt(argc, argv, "dht:vw:")) != -1) {
		switch (opt) {
			case 'd':
				flags = true;
				break;
			case 't':
				threads = atoi(optarg);
				break;
			case 'w':
				snprintf(workspace, PATH_MAX, "%s", optarg);
				break;
			case 'h':
				fprintf(stdout, "Usage: %s [-d] [-h] [-t threads] [-v] [-w workspace]\n", argv[0]);
				return EXIT_SUCCESS;
			case 'v':
				fprintf(stdout, "%s", banner);
				return EXIT_SUCCESS;
			default:
				return EXIT_FAILURE;
		}
	}

	log4cplus::initialize();
	log4cplus::ConfigureAndWatchThread configureAndWatchThread(LOG4CPLUS_TEXT ("log4cplus.properties"));

	LOG4CPLUS_INFO(root, banner);
	LOG4CPLUS_INFO(root, "threads: " << threads << ", workspace: " << workspace);

	int r;
	if (flags)
	{
		int nochdir = 0;
		int noclose = 0;
		r = daemon(nochdir, noclose);
		if (r == -1)
		{
			fprintf(stderr, "daemon: %s(%d)", strerror(errno), errno);
			return EXIT_FAILURE;
		}
	}

	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	zmq_ctx = zmq_ctx_new ();
	if (zmq_ctx == NULL)
	{
		return EXIT_FAILURE;
	}

	int type = ZMQ_ROUTER;
	router = zmq_socket (zmq_ctx, type);
	if (router == NULL)
	{
		LOG4CPLUS_ERROR(root, "zmq_socket: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return EXIT_FAILURE;
	}

	type = ZMQ_DEALER;
	dealer = zmq_socket (zmq_ctx, type);
	if (dealer == NULL)
	{
		LOG4CPLUS_ERROR(root, "zmq_socket: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return EXIT_FAILURE;
	}

	char endpoint[PATH_MAX];
	strcpy(endpoint, "tcp://0.0.0.0:49001");
	r = zmq_bind (router, endpoint);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_bind: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		snaild_term();
		return EXIT_FAILURE;
	}

	strcpy(endpoint, "inproc://snaild");
	r = zmq_bind (dealer, endpoint);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_bind: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		snaild_term();
		return EXIT_FAILURE;
	}

	int resource = RLIMIT_NPROC;
	struct rlimit rlim;
	r = getrlimit(resource, &rlim);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "getrlimit: " << strerror(errno) << "(" << errno << ")");
		snaild_term();
		return EXIT_FAILURE;
	}

	pthread_t hiredis;
	r = pthread_create(&hiredis, NULL, hiredis_routine, NULL);
	if (r != 0)
	{
		LOG4CPLUS_ERROR(root, "pthread_create: " << strerror(errno) << "(" << errno << ")");
		snaild_term();
		return EXIT_FAILURE;
	}

	int min_rlim = (rlim.rlim_cur < threads)? rlim.rlim_cur: threads;
	LOG4CPLUS_INFO(root, "RLIMIT_NPROC {rlim_cur: "
			<< rlim.rlim_cur << ", rlim_max: "
			<< rlim.rlim_max << "} -> min_rlim: "
			<< min_rlim);

	for (int i = 0; i < min_rlim; i++)
	{
		pthread_t snaild;
		r = pthread_create(&snaild, NULL, snaild_routine, zmq_ctx);
		if (r != 0)
		{
			LOG4CPLUS_ERROR(root, "pthread_create: " << strerror(errno) << "(" << errno << ")");
			return EXIT_FAILURE;
		}
	}

	void *frontend = router;
	void *backend = dealer;
	void *capture = NULL;
	r = zmq_proxy (frontend, backend, capture);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_proxy: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		snaild_term();
		return EXIT_FAILURE;
	}
	return snaild_term();
}

void *hiredis_routine(void *arg)
{
	(void)arg;

	// TODO: tmp code
	const char *ip = "127.0.0.1";
	int port = 6379;
	redisContext *hiredis_ctx = redisConnect(ip, port);
	if (hiredis_ctx == NULL || hiredis_ctx->err)
	{
	  if (hiredis_ctx != NULL)
	  {
	    LOG4CPLUS_ERROR(root, "redisConnect: " << hiredis_ctx->errstr);
	  }
	  else
	  {
	    LOG4CPLUS_ERROR(root, "redisContext is " << hiredis_ctx);
	  }
		log4cplus::threadCleanup();
	  return arg;
	}

	// TODO: add event loop
	const char *command = "KEYS *";
	redisReply *reply = (redisReply *)redisCommand(hiredis_ctx, command);
	if (reply == NULL)
	{
	  LOG4CPLUS_ERROR(root, "redisConnect: " << hiredis_ctx->errstr);
		redisFree(hiredis_ctx);
		log4cplus::threadCleanup();
	  return arg;
	}

	LOG4CPLUS_INFO(root, "command: " << command);
	LOG4CPLUS_INFO(root, "type: " << reply->type << ", integer: " << reply->integer << ", str: " << reply->str);
	for (size_t i = 0; i < reply->elements; i++)
	{
		LOG4CPLUS_INFO(root, "[" << i << "/" << reply->elements << "] type: " << reply->element[i]->type
										<< ", integer: " << reply->element[i]->integer
										<< ", str: " << reply->element[i]->str);
	}
	freeReplyObject(reply);
	redisFree(hiredis_ctx);

	log4cplus::threadCleanup();
	return arg;
}

void *snaild_routine(void *arg)
{
	void *zmq_ctx = arg;
	int type = ZMQ_REP;
	void *snaild = zmq_socket (zmq_ctx, type);
	if (snaild == NULL)
	{
		LOG4CPLUS_ERROR(root, "zmq_socket: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		log4cplus::threadCleanup();
		pthread_exit(NULL);
	}

	char endpoint[PATH_MAX];
	strcpy(endpoint, "inproc://snaild");
	int r = zmq_connect (snaild, endpoint);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_connect: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		log4cplus::threadCleanup();
		pthread_exit(NULL);
	}

	while (true)
	{
		zmq_msg_t msg;
		r = zmq_msg_init (&msg);
		if (r == -1)
		{
			LOG4CPLUS_ERROR(root, "zmq_msg_init: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
			continue;
		}

		int flags = 0;
		int rr = zmq_msg_recv (&msg, snaild, flags);
		if (rr == -1)
		{
			LOG4CPLUS_ERROR(root, "zmq_msg_recv: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		}
		else
		{
			void *msg_content = zmq_msg_data (&msg);
			LOG4CPLUS_INFO(root, "snaild: " << snaild << ", zmq_msg_recv: " << msg_content << ", rr: " << rr);
			received_message(snaild, msg_content, rr);
			r = zmq_msg_close (&msg);
			if (r == -1)
			{
				LOG4CPLUS_ERROR(root, "zmq_msg_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
			}
		}
	}

	log4cplus::threadCleanup();
	return arg;
}

void received_message(void *socket, void *receivedBuffer, int receivedBufferSize)
{
	snailp::TransferRequest transferRequest;
	bool rb = transferRequest.ParseFromArray(receivedBuffer, receivedBufferSize);
	if (!rb)
	{
		return;
	}

	snailp::TransferRespond transferRespond;
	rb = fragment_to_file(transferRequest, transferRespond);
	if (!rb)
	{
		LOG4CPLUS_ERROR(root, "fragment_to_file is wrong!");
	}

	size_t size = transferRespond.ByteSizeLong();
	void *send_buffer = tc_malloc(size);
	if (send_buffer == NULL)
	{
		LOG4CPLUS_ERROR(root, "tc_malloc: " << strerror(errno) << "(" << errno << ")");
		return;
	}

	rb = transferRespond.SerializeToArray(send_buffer, size);
	if (!rb)
	{
		tc_free(send_buffer);
		return;
	}

	zmq_msg_t sendmsg;
	int r = zmq_msg_init_data (&sendmsg, send_buffer, size, my_free, NULL);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_msg_init_data: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		tc_free(send_buffer);
		return;
	}

	void *snaild = socket;
	int flags = 0;
	int rs = zmq_sendmsg (snaild, &sendmsg, flags);
	if (rs == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_sendmsg: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return;
	}
	LOG4CPLUS_INFO(root, "snaild: " << snaild << ", zmq_sendmsg: " << send_buffer << ", rs: " << rs);

	r = zmq_msg_close (&sendmsg);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_msg_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return;
	}
}

bool fragment_to_file(const snailp::TransferRequest &transferRequest, snailp::TransferRespond &transferRespond)
{
	bool rb = true;
	int errnum_ = 0;

	for (int i = 0; i < transferRequest.fragment_size(); i++)
	{
		const snailp::TransferRequest_Fragment &fragment = transferRequest.fragment(i);

		uLong sourceLen = fragment.ptr().length();
		const Bytef *source = (const Bytef *)fragment.ptr().c_str();

		uLong crc = crc32(0L, source, (uInt)sourceLen);
		if (crc != fragment.crc32())
		{
			errnum_ = -100;
			LOG4CPLUS_ERROR(root, "crc(" << crc << ") not match: " << fragment.crc32());
			rb = false;
			break;
		}

		char name[PATH_MAX];
		snprintf(name, PATH_MAX, "%s", fragment.name().c_str());

		char temp_path[PATH_MAX];
		snprintf(temp_path, PATH_MAX, "%s/%s.snailp", workspace, name);

		char rawpath[PATH_MAX];
		char command[PATH_MAX];
		strcpy(rawpath, temp_path);
		snprintf(command, PATH_MAX, "mkdir -p %s", dirname(rawpath));
		int r = system(command);
		if (r != 0)
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "system: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		FILE *fp = fopen(temp_path, "ab");
		if (fp == NULL)
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "fopen: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		r = fseeko(fp, 0, SEEK_END);
		if (r == -1)
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "fseeko: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		off_t offset = fragment.offset();
		off_t told = ftello(fp);
		if (told == -1)
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "ftello: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		size_t size = sizeof (char);
		size_t nmemb;
		size_t rfw;
		uLongf destLen = fragment.plain();
		void *dest = tc_malloc(destLen);
		if (dest == NULL)
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "tc_malloc: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		r = uncompress ((Bytef *)dest, &destLen, source, sourceLen);
		if (r != Z_OK)
		{
			tc_free(dest);
			errnum_ = r;
			LOG4CPLUS_ERROR(root, "r: " << r << ", uncompress returns Z_OK(" << Z_OK << ") if success, "
					"Z_MEM_ERROR(" << Z_MEM_ERROR << ") if there was not enough memory, "
					"Z_BUF_ERROR(" << Z_BUF_ERROR << ") if there was not enough room in the output "
					"buffer, or Z_DATA_ERROR(" << Z_DATA_ERROR << ") if the input data was corrupted "
					"or incomplete.  In the case where there is not enough room, uncompress() will "
					"fill the output buffer with the uncompressed data up to that point.");
			rb = false;
			break;
		}
		LOG4CPLUS_DEBUG(root, "Decompress rate: " << 100 * (destLen >= sourceLen ? destLen - sourceLen : 0) / destLen
				<< "%, sourceLen: " << sourceLen << ", destLen: " << destLen);

		nmemb = destLen;
		ssize_t guess = offset + size * nmemb;
		if (told >= guess)
		{
			LOG4CPLUS_WARN(root, "ignore fragment, told: " << told << ", guess: " << guess);
			rfw = size * nmemb;
		}
		else if (told >= offset && told < guess)
		{
			size_t errata = told - offset;
			if (errata > 0)
			{
				LOG4CPLUS_WARN(root, "errata: " << errata);
			}
			rfw = fwrite(dest + errata, size, nmemb - errata, fp);
			rfw += errata;
		}
		else
		{
			__set_errno (ESPIPE);
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "[?]: " << strerror(errno) << "(" << errno << ")");
			rb = false;
		}
		tc_free(dest);

		r = fclose(fp);
		if (r == EOF)
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "fclose: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		if (!rb)
		{
			break;
		}

		if (rfw < size * nmemb)
		{
			__set_errno (ENOSPC);
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "[?]: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		if (fragment.eof())
		{
			char new_path[PATH_MAX];
			snprintf(new_path, PATH_MAX, "%s/%s", workspace, name);
			r = rename(temp_path, new_path);
			if (r == -1)
			{
				errnum_ = errno;
				LOG4CPLUS_ERROR(root, "rename: " << strerror(errno) << "(" << errno << ")");
				rb = false;
				break;
			}
		}

		LOG4CPLUS_INFO(root, "[" << i << "]: name: '" << fragment.name().c_str()
				<< "', offset: " << offset
				<< ", dest: " << dest << "(" << rfw << "), crc32: "
				<< fragment.crc32() << ", eof: " << fragment.eof());
	}

	std::string errstring_;
	if (errnum_ >= 0)
	{
		errstring_ = strerror(errnum_);
	}

	transferRespond.set_seq(transferRequest.seq());

	pid_t tid = syscall(SYS_gettid);
	transferRespond.set_tid(tid);

	struct tm tm_;
	struct timeval tv_;
	gettimeofday(&tv_, NULL);
	localtime_r(&tv_.tv_sec, &tm_);
	int64_t currentMSecsSinceEpoch = (int64_t)tv_.tv_sec * 1000 + tv_.tv_usec / 1000;

	transferRespond.set_created(currentMSecsSinceEpoch);
	transferRespond.set_errnum((int64_t)errnum_);
	transferRespond.set_errstring(errstring_);
	return true;
}

int snaild_term()
{
	int r = zmq_close (dealer);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return EXIT_FAILURE;
	}

	r = zmq_close (router);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return EXIT_FAILURE;
	}

	r = zmq_ctx_term (zmq_ctx);
	if (r == -1)
	{
		LOG4CPLUS_ERROR(root, "zmq_ctx_term: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return EXIT_FAILURE;
	}

	log4cplus::Logger::shutdown();
	return EXIT_SUCCESS;
}
