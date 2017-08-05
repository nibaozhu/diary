#include <unistd.h>
#include <cstdlib>
#include <glog/logging.h>

int main(int argc, char* argv[]) {
	// Initialize Google's logging library.
	google::InitGoogleLogging(argv[0]);

	// ...
	srand(getpid());
	size_t num_cookies = rand();
	LOG(INFO) << "Found " << num_cookies << " cookies";
	LOG(WARNING) << "Found " << num_cookies << " cookies";
	LOG(ERROR) << "Found " << num_cookies << " cookies";
	// LOG(FATAL) << "Found " << num_cookies << " cookies";
}
