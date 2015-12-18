#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
extern int h_errno;

#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

int main() {
    const char ip_string[] = "127.0.0.1";

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    int ret = inet_pton(AF_INET, ip_string, (struct sockaddr *) &addr.sin_addr.s_addr);
    if (ret == 0) {
        printf("%s(%d)\n", strerror(errno), errno);
        return 1;
    }

    struct hostent *ht = NULL;
    socklen_t len = sizeof (addr);
    int type = AF_INET;

    ht = gethostbyaddr((const void *)&(addr.sin_addr.s_addr), len, type);
    if (ht == NULL) 
        printf("%s(%d)\n", hstrerror(h_errno), h_errno);
    else
        printf("ht = %p\n", ht);
    return 0;
}
