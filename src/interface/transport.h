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
	Transport(int fd, int size = 1024 * 1024) {
		memset(this, 0, sizeof *this);
		this->fd = fd;
		if (size <= 0) {
			size = 1024 * 1024; /* default 1024 * 1024 bytes (1 MB) */
		}
		this->size = size;
		data = malloc(this->size);
		memset(data, 0, this->size);
	}

	int set_fd(int fd) {
		return this->fd = fd;
	}

	int get_fd(void) {
		return this->fd;
	}

	void *set_data(void *data, int size) {
		if (this->size < size) {
			return NULL;
		}
		memcpy(this->data + this->position, data, size);
		this->position += size;
		return this->data + this->position;
	}

	void *get_data(void) {
		return this->data;
	}

	int set_size(int size) {
		return this->size = size;
	}

	int get_size(void) {
		return this->size;
	}

	~Transport() {
		free(this->data);
		memset(this, 0, sizeof *this);
	}
};

