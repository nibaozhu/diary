#include <ctime>
#include <cstring>
#include <cstdlib>

class Transport {
private:
	time_t created; /* the first communication time */
	time_t updated; /* the lastest communication time */
	bool alive; /* true: live; false: die */
	int fd; /* file descriptor */
	void *data; /* transport data */
	int position; /* transport data pointer position */
	int size; /* transport data size */
	double speed; /* bytes per second */

public:
	Transport() {
		memset(this, 0, sizeof *this);
	}

	int set_fd(int fd) {
		return this->fd = fd;
	}

	int get_fd(void) {
		return this->fd;
	}

	~Transport() {
		free(this->data);
		memset(this, 0, sizeof *this);
	}
};

