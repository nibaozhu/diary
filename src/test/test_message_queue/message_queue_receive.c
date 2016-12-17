#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
    long mtype;       /* message type, must be > 0 */
    char mtext[64];    /* message data */
};

int main(int argc, char **argv)
{
	FILE *stream = stdout;
	unsigned int seed = (unsigned int)getpid();
	srand(seed);

	const char *pathname = ".";
	int proj_id = 1;
	key_t key = ftok(pathname, proj_id);

	fprintf(stream, "ftok: key=0x%08x, pathname: %s, proj_id = %d, errno=%d, strerror: %s\n", key, pathname, proj_id, errno, strerror(errno));

	int msgflg = IPC_CREAT;
	int msqid = msgget(key, msgflg);

	fprintf(stream, "msgget: msqid=%d, errno=%d, strerror: %s\n", msqid, errno, strerror(errno));

	if (msqid == -1) {
		return -1;
	}

	int cmd = IPC_STAT;
	struct msqid_ds *buf = (struct msqid_ds *)malloc(sizeof (struct msqid_ds));
	int ret = msgctl(msqid, cmd, buf);

	fprintf(stream, "msgctl: ret=%d, msqid=%d, cmd=%d, buf=%p, errno=%d, strerror: %s\n", ret, msqid, cmd, buf, errno, strerror(errno));

	if (ret == -1) {
		return -1;
	}

	cmd = IPC_SET;
	int msgctl(int msqid, int cmd, struct msqid_ds *buf);

	struct msgbuf *msgp = (struct msgbuf*)malloc(sizeof (struct msgbuf));
	memset(msgp, 0, sizeof (struct msgbuf));
	msgp->mtype = (long)rand();

	size_t msgsz = sizeof (struct msgbuf) - sizeof (long);
	int i = 0;

#if 0
	do {
		char string0[64] = {0};
		sprintf(string0, "hello,rand=0x%08x", rand());
		strcpy(msgp->mtext, string0);

		ret = msgsnd(msqid, (const void *)msgp, msgsz, msgflg);
		fprintf(stream, "msgsnd: i=%d, ret=%d, msqid=%d, msgp=%p, msgp->mtext: %s, msgsz=%d, msgflg=%d, errno=%d, strerror: %s\n", i++, ret, msqid, msgp, msgp->mtext, msgsz, msgflg, errno, strerror(errno));

		if (ret == -1) {
			return -1;
		}
	} while (i < 5);
#endif

	i = 0;
	ssize_t sret = 0;
	do {
		memset(msgp->mtext, 0, msgsz);
		long msgtyp = 0;
		sret = msgrcv(msqid, (void *)msgp, msgsz, msgtyp, msgflg);
		fprintf(stream, "msgrcv: i=%d, sret=%d, msqid=%d, msgp=%p, msgp->mtext: %s, msgsz=%d, msgflg=%d, errno=%d, strerror: %s\n", i++, sret, msqid, msgp, msgp->mtext, msgsz, msgflg, errno, strerror(errno));

		if (sret == -1) {
			return -1;
		}
	} while (1);

	return 0;
}

