#include <stdio.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>


typedef struct configure_s {
        const char *path; /* # chars in a path name including nul */
        pid_t pid;

        int (*fp)(void);
} configure_t;

int main(int argc, char **argv) {
        int ret = 0;

#if 0
        void *dlopen(const char *filename, int flag);
        
        char *dlerror(void);
        
        void *dlsym(void *handle, const char *symbol);
        
        int dlclose(void *handle);
        
        Link with -ldl.
#endif

        do {
                if (argc <= 1) {
                        fprintf(stderr, "%s filename\n", argv[0]);
                        break;
                }

                const char *filename = argv[1];
                int flag = RTLD_NOW;
                void *handle = NULL;
                const char *stringerror = NULL;

                handle = dlopen(filename, flag);
                stringerror = dlerror();
                if (handle == NULL || stringerror != NULL) {
                        fprintf(stderr, "handle = %p, %s\n", handle, stringerror);
                        dlerror();
                        break;
                }

                const char *symbol = "g_conf";
                configure_t *address;
                *(void **) &address = dlsym(handle, symbol);
                stringerror = dlerror();
                if (address == NULL || stringerror != NULL) {
                        fprintf(stderr, "handle = %p, %s\n", address, stringerror);
                        dlerror();
                }

                address->fp();
                address->path = argv[0];
                fprintf(stdout, "address = %p, address->pid = %d, address->path = %s\n", address, address->pid, address->path);
        } while (0);

        return ret;
}
