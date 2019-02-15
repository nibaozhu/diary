// Author: Baozhu Ni
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <paths.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <gperftools/tcmalloc.h>
#include <hiredis/hiredis.h>
#include <zlib.h>
#include <zmq.h>

#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/version.h>

#include "hummingbirdp.pb.h"

#ifndef __set_errno
# define __set_errno(Val) errno = (Val)
#endif

#if defined __GNUC__
#define likely(x) __builtin_expect ((x), 1)
#define unlikely(x) __builtin_expect ((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

log4cplus::Logger root = log4cplus::Logger::getRoot();
void *dealer;
void *router;
void *zmq_ctx;

int threads;
int nest_port = 49001;
char workspace[PATH_MAX] = _PATH_TMP;
bool linkable = false;

int redis_port = 6379;
char redis_host[NAME_MAX] = "127.0.0.1";

void my_free(void *data, void *hint)
{
	(void)hint;
	tc_free(data);
}

void *nest_routine(void *arg);
void received_message(void *nest, void *receivedBuffer, int receivedBufferSize, redisContext *hiredis_ctx);
bool fragment_to_file(const hummingbirdp::Request &request, hummingbirdp::Respond &respond, redisContext *hiredis_ctx);
bool hummingbirdp_cached(redisContext *hiredis_ctx, const char *distinct, const char *new_path);
bool hummingbirdp_cached_ctrl(redisContext *hiredis_ctx, const char *command, const char *nest_hash_field, const char *nest_hash_value);
void print_stat(struct stat &sb);
int nest_term();

int main(int argc, char **argv)
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	char current_dir_name[PATH_MAX];
	size_t current_dir_name_size = PATH_MAX;
	char *rdir = getcwd(current_dir_name, current_dir_name_size);
	if (unlikely(rdir == NULL))
	{
		fprintf(stderr, "getcwd: %s(%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}
	char property_file[PATH_MAX];
	snprintf(property_file, PATH_MAX, "%s/log4cplus.properties", current_dir_name);

	int mode = F_OK | R_OK;
	int r = access(property_file, mode);
	if (unlikely(r == -1))
	{
		fprintf(stderr, "access: %s(%d), property_file: \"%s\"\n", strerror(errno), errno, property_file);
		return EXIT_FAILURE;
	}

	int zmq_major;
	int zmq_minor;
	int zmq_patch;
	zmq_version (&zmq_major, &zmq_minor, &zmq_patch);

	char banner[NAME_MAX];
	snprintf(banner, NAME_MAX, "%s %s (compiled %s %s), %s, hiredis %d.%d.%d, log4cplus %s, protobuf %s, zlib %s, zmq %d.%d.%d",
			argv[0], NEST_VERSION_STRING, __DATE__, __TIME__,
			TC_VERSION_STRING, HIREDIS_MAJOR, HIREDIS_MINOR, HIREDIS_PATCH,
			LOG4CPLUS_VERSION_STR, google::protobuf::internal::VersionString(GOOGLE_PROTOBUF_VERSION).c_str(),
			zlib_version, zmq_major, zmq_minor, zmq_patch);
	bool flags = false;
	int opt;
	while ((opt = getopt(argc, argv, "dhlp:t:vw:H:P:")) != -1) {
		switch (opt) {
			case 'd':
				flags = true;
				break;
			case 'h':
				fprintf(stdout, "Usage: %s [-d] [-h] [-l] [-p port] [-t threads] [-v] [-w workspace] [-H redis_host] [-P redis_port]\n", argv[0]);
				return EXIT_SUCCESS;
			case 'l':
				linkable = true; 
				break;
			case 'p':
				nest_port = atoi(optarg);
				break;
			case 't':
				threads = atoi(optarg);
				break;
			case 'v':
				fprintf(stdout, "%s\n", banner);
				return EXIT_SUCCESS;
			case 'w':
				snprintf(workspace, PATH_MAX, "%s", optarg);
				break;
			case 'H':
				strcpy(redis_host, optarg);
				break;
			case 'P':
				redis_port = atoi(optarg);
				break;
			default:
				return EXIT_FAILURE;
		}
	}

	if (unlikely(threads <= 0))
	{
		int name = _SC_NPROCESSORS_ONLN;
		long _sc_nprocessors_onln = sysconf(name);
		if (unlikely(_sc_nprocessors_onln == -1))
		{
			fprintf(stderr, "sysconf: %s(%d), _SC_NPROCESSORS_ONLN: %d\n", strerror(errno), errno, _SC_NPROCESSORS_ONLN);
			return EXIT_FAILURE;
		}
	
		if (likely(_sc_nprocessors_onln > 0))
		{
			threads = _sc_nprocessors_onln;
		}
		else
		{
			threads = ZMQ_POLLITEMS_DFLT;
		}
	}

	if (likely(strlen(workspace) > 0))
	{
		size_t len = strlen(workspace);
		for (size_t i = len - 1; i > 0; i--)
		{
			if (likely(workspace[i] != '/'))
			{
				workspace[i+1] = '/';
				break;
			}
		}
	}

	mode = F_OK | R_OK | W_OK | X_OK;
	r = access(workspace, mode);
	if (unlikely(r == -1))
	{
		fprintf(stderr, "access: %s(%d), workspace: \"%s\"\n", strerror(errno), errno, workspace);
		return EXIT_FAILURE;
	}

	if (likely(flags))
	{
		int nochdir = 0;
		int noclose = 0;
		r = daemon(nochdir, noclose);
		if (unlikely(r == -1))
		{
			fprintf(stderr, "daemon: %s(%d)\n", strerror(errno), errno);
			return EXIT_FAILURE;
		}
	}

	log4cplus::initialize();
	log4cplus::helpers::LogLog::getLogLog()->setInternalDebugging(true);
	log4cplus::ConfigureAndWatchThread configureAndWatchThread(LOG4CPLUS_TEXT (property_file));

	LOG4CPLUS_INFO(root, banner);
	LOG4CPLUS_INFO(root, "current_dir_name: \"" << current_dir_name << "\""
									<< ", property_file: \"" << property_file << "\""
									<< ", port: " << nest_port
									<< ", threads: " << threads
									<< ", workspace: \"" << workspace << "\"");

	zmq_ctx = zmq_ctx_new ();
	if (unlikely(zmq_ctx == NULL))
	{
		LOG4CPLUS_ERROR(root, "zmq_ctx_new: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		nest_term();
		return EXIT_FAILURE;
	}

	int type = ZMQ_ROUTER;
	router = zmq_socket (zmq_ctx, type);
	if (unlikely(router == NULL))
	{
		LOG4CPLUS_ERROR(root, "zmq_socket: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		nest_term();
		return EXIT_FAILURE;
	}

	type = ZMQ_DEALER;
	dealer = zmq_socket (zmq_ctx, type);
	if (unlikely(dealer == NULL))
	{
		LOG4CPLUS_ERROR(root, "zmq_socket: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return EXIT_FAILURE;
	}

	char endpoint[PATH_MAX];
	snprintf(endpoint, PATH_MAX, "tcp://0.0.0.0:%d", nest_port);
	r = zmq_bind (router, endpoint);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_bind: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		nest_term();
		return EXIT_FAILURE;
	}

	strcpy(endpoint, "inproc://nest");
	r = zmq_bind (dealer, endpoint);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_bind: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		nest_term();
		return EXIT_FAILURE;
	}

	for (int i = 0; i < threads; i++)
	{
		pthread_t nest;
		r = pthread_create(&nest, NULL, nest_routine, zmq_ctx);
		if (unlikely(r != 0))
		{
			LOG4CPLUS_ERROR(root, "pthread_create: " << strerror(errno) << "(" << errno << ")");
			return EXIT_FAILURE;
		}
	}

	void *frontend = router;
	void *backend = dealer;
	void *capture = NULL;
	r = zmq_proxy (frontend, backend, capture);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_proxy: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		nest_term();
		return EXIT_FAILURE;
	}
	return nest_term();
}

void *nest_routine(void *arg)
{
	void *zmq_ctx = arg;
	int type = ZMQ_REP;
	void *nest = zmq_socket (zmq_ctx, type);
	if (unlikely(nest == NULL))
	{
		LOG4CPLUS_ERROR(root, "zmq_socket: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		log4cplus::threadCleanup();
		pthread_exit(NULL);
	}

	char endpoint[PATH_MAX];
	strcpy(endpoint, "inproc://nest");
	int r = zmq_connect (nest, endpoint);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_connect: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		log4cplus::threadCleanup();
		pthread_exit(NULL);
	}

	redisContext *hiredis_ctx = redisConnect(redis_host, redis_port);
	if (unlikely(hiredis_ctx == NULL || hiredis_ctx->err))
	{
		if (unlikely(hiredis_ctx != NULL))
		{
			LOG4CPLUS_ERROR(root, "redisConnect: " << hiredis_ctx->errstr);
		}
		else
		{
			LOG4CPLUS_ERROR(root, "redisContext is " << hiredis_ctx);
		}
		log4cplus::threadCleanup();
		pthread_exit(NULL);
	}

	while (true)
	{
		zmq_msg_t msg;
		r = zmq_msg_init (&msg);
		if (unlikely(r == -1))
		{
			LOG4CPLUS_ERROR(root, "zmq_msg_init: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
			continue;
		}

		int flags = 0;
		int rr = zmq_msg_recv (&msg, nest, flags);
		if (unlikely(rr == -1))
		{
			LOG4CPLUS_ERROR(root, "zmq_msg_recv: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		}
		else
		{
			void *msg_content = zmq_msg_data (&msg);
			LOG4CPLUS_DEBUG(root, "nest: " << nest << ", zmq_msg_recv: " << msg_content << ", rr: " << rr);
			received_message(nest, msg_content, rr, hiredis_ctx);
			r = zmq_msg_close (&msg);
			if (unlikely(r == -1))
			{
				LOG4CPLUS_ERROR(root, "zmq_msg_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
			}
		}
	}

	redisFree(hiredis_ctx);
	log4cplus::threadCleanup();
	return arg;
}

void received_message(void *nest, void *receivedBuffer, int receivedBufferSize, redisContext *hiredis_ctx)
{
	hummingbirdp::Request request;
	bool rb = request.ParseFromArray(receivedBuffer, receivedBufferSize);
	if (unlikely(!rb))
	{
		return;
	}

	hummingbirdp::Respond respond;
	rb = fragment_to_file(request, respond, hiredis_ctx);
	if (unlikely(!rb))
	{
		LOG4CPLUS_ERROR(root, "fragment_to_file is wrong!");
	}

	size_t size = respond.ByteSizeLong();
	void *send_buffer = tc_malloc(size);
	if (unlikely(send_buffer == NULL))
	{
		LOG4CPLUS_ERROR(root, "tc_malloc: " << strerror(errno) << "(" << errno << ")");
		return;
	}

	rb = respond.SerializeToArray(send_buffer, size);
	if (unlikely(!rb))
	{
		tc_free(send_buffer);
		return;
	}

	zmq_msg_t sendmsg;
	int r = zmq_msg_init_data (&sendmsg, send_buffer, size, my_free, NULL);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_msg_init_data: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		tc_free(send_buffer);
		return;
	}

	int flags = 0;
	int rs = zmq_sendmsg (nest, &sendmsg, flags);
	if (unlikely(rs == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_sendmsg: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return;
	}
	LOG4CPLUS_DEBUG(root, "nest: " << nest << ", zmq_sendmsg: " << send_buffer << ", rs: " << rs);

	r = zmq_msg_close (&sendmsg);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_msg_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
		return;
	}
}

bool fragment_to_file(const hummingbirdp::Request &request, hummingbirdp::Respond &respond, redisContext *hiredis_ctx)
{
	bool rb = true;
	int errnum_ = 0;

	for (int i = 0; i < request.fragment_size(); i++)
	{
		const hummingbirdp::Request_Fragment &fragment = request.fragment(i);

		char name[PATH_MAX];
		snprintf(name, PATH_MAX, "%s", fragment.name().c_str());

		char temp_path[PATH_MAX];
		snprintf(temp_path, PATH_MAX, "%s%s~", workspace, fragment.path().c_str());

		char new_path[PATH_MAX];
		snprintf(new_path, PATH_MAX, "%s%s", workspace, fragment.path().c_str());

		uLong sourceLen = fragment.ptr().length();
		const Bytef *source = (const Bytef *)fragment.ptr().c_str();

		if (likely(fragment.crc32() != 0))
		{
			uLong crc = crc32(0L, source, (uInt)sourceLen);
			if (unlikely(crc != fragment.crc32()))
			{
				errnum_ = -100;
				LOG4CPLUS_ERROR(root, "crc(" << crc << ") not match: " << fragment.crc32());
				rb = false;
				break;
			}
		}

		char rawpath[PATH_MAX];
		char command[PATH_MAX];
		strcpy(rawpath, temp_path);

		for (size_t j = 0; j < PATH_MAX; j++)
		{
			if (rawpath[j] == '\\')
			{
				rawpath[j] = '/';
			}
		}
		snprintf(command, PATH_MAX, "mkdir -p \"%s\"", dirname(rawpath));
		int r = system(command);
		if (unlikely(r != 0 && WIFEXITED(r) != 0))
		{
			__set_errno (WEXITSTATUS(r));
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "system: " << strerror(errno) << "(" << errno << ")" << ": command: " << command);
			rb = false;
			break;
		}

		if (unlikely(fragment.offset() == 0))
		{
			bool rb0 = hummingbirdp_cached(hiredis_ctx, fragment.distinct().c_str(), new_path);
			if (unlikely(rb0))
			{
				LOG4CPLUS_INFO(root, "[" << i << "]: name: \"" << fragment.name().c_str()
												<< "\", distinct: \"" << fragment.distinct().c_str()
												<< "\", new_path: \"" << new_path << "\"");

				hummingbirdp::Respond_CopyOnWrite *pcopyonwrite = respond.add_copyonwrite();
				pcopyonwrite->set_name(fragment.name());
				pcopyonwrite->set_path(fragment.path());
				pcopyonwrite->set_distinct(fragment.distinct());
				continue;
			}
		}

		FILE *fp = fopen(temp_path, "ab");
		if (unlikely(fp == NULL))
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "fopen: " << strerror(errno) << "(" << errno << ")" << ": temp_path: " << temp_path);
			rb = false;
			break;
		}

		r = fseeko(fp, 0, SEEK_END);
		if (unlikely(r == -1))
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "fseeko: " << strerror(errno) << "(" << errno << ")" << ": fp: " << fp);
			rb = false;
			break;
		}

		off_t offset = fragment.offset();
		off_t told = ftello(fp);
		if (unlikely(told == -1))
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "ftello: " << strerror(errno) << "(" << errno << ")" << ": fp: " << fp);
			rb = false;
			break;
		}

		uLongf destLen = fragment.srclen();
		void *dest = tc_malloc(destLen);
		if (unlikely(dest == NULL))
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "tc_malloc: " << strerror(errno) << "(" << errno << ")" << ": destLen: " << destLen);
			rb = false;
			break;
		}

		r = uncompress ((Bytef *)dest, &destLen, source, sourceLen);
		if (unlikely(r != Z_OK))
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

		bool ignore = false;
		size_t size = sizeof (char);
		size_t rfw = 0;
		size_t nmemb = destLen;
		ssize_t guess = offset + size * nmemb;
		if (unlikely(told > guess && guess > 0))
		{
			LOG4CPLUS_WARN(root, "ignore fragment, told: " << told << ", guess: " << guess);
			rfw = size * nmemb;
		}
		else if (told >= offset && told <= guess)
		{
			size_t errata = told - offset;
			if (unlikely(errata > 0))
			{
				LOG4CPLUS_WARN(root, "errata: " << errata);
			}
			rfw = errata;
			rfw += fwrite((void*)((char*)dest + errata), size, nmemb - errata, fp);
		}
		else
		{
			int mode = F_OK | R_OK | W_OK;
			r = access(new_path, mode);
			if (unlikely(r == -1))
			{
				LOG4CPLUS_WARN(root, "access: " << strerror(errno) << "(" << errno << ")" << ": new_path: " << new_path);

				__set_errno (ESPIPE);
				errnum_ = errno;
				LOG4CPLUS_ERROR(root, "[?]: " << strerror(errno) << "(" << errno << ")");
				rb = false;
			}
			else
			{
				ignore = true;
				LOG4CPLUS_WARN(root, "ignore fragment, new_path: " << new_path << ", mode: 0" << std::oct << mode);
				rfw = size * nmemb;
			}
		}
		tc_free(dest);

		uInt rate = 0;
		if (likely(destLen > 0))
		{
			rate = 100 * (destLen > sourceLen ? destLen - sourceLen : 0) / destLen;
		}
		LOG4CPLUS_INFO(root, "[" << i << "]: name: \"" << fragment.name().c_str()
				<< "\", temp_path: \"" << temp_path
				<< "\", offset: " << offset
				<< ", uncompress rate: " << rate
				<< "%, sourceLen: " << sourceLen << ", destLen: " << destLen
				<< ", dest: " << dest << "(rfw: " << rfw << "), crc32: 0x"
				<< std::hex << fragment.crc32() << ", eof: "
				<< std::dec << fragment.eof());

		r = fclose(fp);
		if (unlikely(r == EOF))
		{
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "fclose: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		if (unlikely(!rb))
		{
			break;
		}

		if (unlikely(rfw < size * nmemb))
		{
			__set_errno (ENOSPC);
			errnum_ = errno;
			LOG4CPLUS_ERROR(root, "[?]: " << strerror(errno) << "(" << errno << ")");
			rb = false;
			break;
		}

		if (unlikely(ignore))
		{
			r = unlink(temp_path);
			if (unlikely(r == -1))
			{
				LOG4CPLUS_WARN(root, "unlink: " << strerror(errno) << "(" << errno << ")" << ": temp_path: " << temp_path);
			}
		}
		else if (unlikely(fragment.eof()))
		{
			char rand_path[PATH_MAX];
			const char *done_path = new_path;
			int mode = F_OK | R_OK | W_OK;
			r = access(new_path, mode);
			if (unlikely(r == 0))
			{
				LOG4CPLUS_WARN(root, "It has been existed, new_path: " << new_path << ", mode: 0" << std::oct << mode);
				snprintf(rand_path, PATH_MAX, "%s%s.%d", workspace, fragment.path().c_str(), rand());
				done_path = rand_path;
			}

			r = rename(temp_path, done_path);
			if (unlikely(r == -1))
			{
				errnum_ = errno;
				LOG4CPLUS_ERROR(root, "rename: " << strerror(errno) << "(" << errno << ")" << ": temp_path: " << temp_path << ", done_path: " << done_path);
				rb = false;
				break;
			}

			const char *nest_hash_field = fragment.distinct().c_str();
			const char *nest_hash_value = new_path;
			hummingbirdp_cached_ctrl(hiredis_ctx, "HSET", nest_hash_field, nest_hash_value);
		}
	}

	std::string errstring_;
	if (likely(errnum_ >= 0))
	{
		errstring_ = strerror(errnum_);
	}

	respond.set_seq(request.seq());

	pid_t tid = syscall(SYS_gettid);
	respond.set_tid(tid);

	struct tm tm_;
	struct timeval tv_;
	gettimeofday(&tv_, NULL);
	localtime_r(&tv_.tv_sec, &tm_);
	int64_t currentMSecsSinceEpoch = (int64_t)tv_.tv_sec * 1000 + tv_.tv_usec / 1000;

	respond.set_created(currentMSecsSinceEpoch);
	respond.set_errnum((int64_t)errnum_);
	respond.set_errstring(errstring_);
	return true;
}

bool hummingbirdp_cached(redisContext *hiredis_ctx, const char *distinct, const char *new_path)
{
	int r;
	const char *nest_hash_field = distinct;
	redisReply *reply = (redisReply *)redisCommand(hiredis_ctx, "HGET nest %s", nest_hash_field);
	if (unlikely(reply == NULL))
	{
		r = redisReconnect(hiredis_ctx);
		LOG4CPLUS_ERROR(root, "redisReconnect: " << hiredis_ctx->errstr << ", r: " << r);
		return false;
	}

	const char *nest_hash_value = new_path;
	if (unlikely(reply->type == REDIS_REPLY_STRING))
	{
		char cached_path[PATH_MAX];
		strcpy(cached_path, reply->str);
		freeReplyObject(reply);

		int mode = F_OK | R_OK | W_OK;
		r = access(cached_path, mode);
		if (unlikely(r == -1))
		{
			hummingbirdp_cached_ctrl(hiredis_ctx, "HDEL", nest_hash_field, nest_hash_value);
			return false;
		}

		struct stat sb;
		r = stat(cached_path, &sb);
		if (unlikely(r == -1))
		{
			hummingbirdp_cached_ctrl(hiredis_ctx, "HDEL", nest_hash_field, nest_hash_value);
			return false;
		}

		print_stat(sb);
		if (sb.st_size <= BUFSIZ)
		{
			LOG4CPLUS_DEBUG(root, "ignore very smaller file, sb.st_size: \"" << sb.st_size << "\"");
			return false;
		}

		if (unlikely(strcmp(cached_path, new_path) == 0))
		{
			LOG4CPLUS_DEBUG(root, "cached_path and new_path are: \"" << new_path << "\"");
			return true;
		}

		if (unlikely(!linkable))
		{
			goto cp;
		}

		r = link(cached_path, new_path);
		if (unlikely(r == -1))
		{
			if (unlikely(errno == ENOTSUP))
			{
				linkable = false;
			}
			LOG4CPLUS_WARN(root, "link: " << strerror(errno) << "(" << errno << ")");

cp:
			char command[PATH_MAX];
			snprintf(command, PATH_MAX, "cp --force \"%s\" \"%s\"", cached_path, new_path);
			int r = system(command);
			if (unlikely(r != 0 && WIFEXITED(r) != 0))
			{
				__set_errno (WEXITSTATUS(r));
				LOG4CPLUS_ERROR(root, "system: " << strerror(errno) << "(" << errno << ")" << ": command: " << command);
				return false;
			}
		}

		hummingbirdp_cached_ctrl(hiredis_ctx, "HSET", nest_hash_field, nest_hash_value);
		return true;
	}
	return false;
}


bool hummingbirdp_cached_ctrl(redisContext *hiredis_ctx, const char *command, const char *nest_hash_field, const char *nest_hash_value)
{
	int r;
	LOG4CPLUS_DEBUG(root, command << " nest \"" << nest_hash_field << "\" \"" << nest_hash_value << "\"");
	redisReply *reply = (redisReply *)redisCommand(hiredis_ctx, "%s nest %s %s", command, nest_hash_field, nest_hash_value);
	if (unlikely(reply == NULL))
	{
		r = redisReconnect(hiredis_ctx);
		LOG4CPLUS_ERROR(root, "redisReconnect: " << hiredis_ctx->errstr << ", r: " << r);
		return false;
	}

	bool rb = true;
	if (likely(reply->type == REDIS_REPLY_INTEGER && reply->integer >= 0))
	{
		LOG4CPLUS_DEBUG(root, "type: " << reply->type << ", integer: " << reply->integer << ", str: " << reply->str);
	}
	else
	{
		LOG4CPLUS_ERROR(root, "type: " << reply->type << ", integer: " << reply->integer << ", str: " << reply->str);
		rb = false;
	}
	freeReplyObject(reply);
	return rb;
}

void print_stat(struct stat &sb)
{
	switch (sb.st_mode & S_IFMT)
	{
		case S_IFBLK:  LOG4CPLUS_DEBUG(root, "block device");            break;
		case S_IFCHR:  LOG4CPLUS_DEBUG(root, "character device");        break;
		case S_IFDIR:  LOG4CPLUS_DEBUG(root, "directory");               break;
		case S_IFIFO:  LOG4CPLUS_DEBUG(root, "FIFO/pipe");               break;
		case S_IFLNK:  LOG4CPLUS_DEBUG(root, "symlink");                 break;
		case S_IFREG:  LOG4CPLUS_DEBUG(root, "regular file");            break;
		case S_IFSOCK: LOG4CPLUS_DEBUG(root, "socket");                  break;
		default:       LOG4CPLUS_DEBUG(root, "unknown?");                break;
	}
	LOG4CPLUS_DEBUG(root, "I-node number:            " << sb.st_ino);
	LOG4CPLUS_DEBUG(root, "Mode:                    0" << std::oct << sb.st_mode);
	LOG4CPLUS_DEBUG(root, "Link count:               " << sb.st_nlink);
	LOG4CPLUS_DEBUG(root, "Ownership:                UID=" << sb.st_uid << "   GID=" << sb.st_gid);
	LOG4CPLUS_DEBUG(root, "Preferred I/O block size: " << sb.st_blksize << " bytes");
	LOG4CPLUS_DEBUG(root, "File size:                " << sb.st_size << " bytes");
	LOG4CPLUS_DEBUG(root, "Blocks allocated:         " << sb.st_blocks);
	LOG4CPLUS_DEBUG(root, "Last status change:       " << ctime(&sb.st_ctime));
	LOG4CPLUS_DEBUG(root, "Last file access:         " << ctime(&sb.st_atime));
	LOG4CPLUS_DEBUG(root, "Last file modification:   " << ctime(&sb.st_mtime));
}

int nest_term()
{
	int r = zmq_close (dealer);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
	}

	r = zmq_close (router);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_close: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
	}

	r = zmq_ctx_term (zmq_ctx);
	if (unlikely(r == -1))
	{
		LOG4CPLUS_ERROR(root, "zmq_ctx_term: " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
	}

	log4cplus::Logger::shutdown();
	return EXIT_SUCCESS;
}
