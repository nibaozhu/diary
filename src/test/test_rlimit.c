#include <sys/time.h>
#include <sys/resource.h>

#if 0
getrlimit() and setrlimit() get and set resource limits respectively.  Each resource has an associated soft and hard limit, as defined by the rlimit structure (the
rlim argument to both getrlimit() and setrlimit()):

    struct rlimit {
        rlim_t rlim_cur;  /* Soft limit */
        rlim_t rlim_max;  /* Hard limit (ceiling for rlim_cur) */
    };
#endif

#if 0
int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);
#endif
int main() {
	// int resource = RLIMIT_NOFILE;
	int resource = RLIMIT_STACK;
	struct rlimit *rlim;
	int ret = getrlimit(resource, rlim);
	return ret;
}
