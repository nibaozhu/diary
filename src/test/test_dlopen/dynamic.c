#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct configure_s {
        const char *path; /* # chars in a path name including nul */
        pid_t pid;

        int (*fp)(void);
} configure_t;


int dlfunc(void);
configure_t g_conf = {NULL, -1, dlfunc};

int dlfunc(void) {
    int ret = 0;
    g_conf.pid = getpid();
    return ret;
}
