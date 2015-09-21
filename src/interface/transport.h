#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

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
		this->data = malloc(this->size);
		memset(data, 0, this->size);
	}

	int set_fd(int fd) {
		return this->fd = fd;
	}

	int get_fd(void) {
		return this->fd;
	}

	int set_position(int position) {
		return this->position = position;
	}

	int get_position(void) {
		return this->position;
	}

	void *set_data(void *data, int size) {
		while (size >= this->size - this->position) {
			printf("realloc %p %d\n", this->data, this->size);
			this->data = realloc(this->data, this->size * 2);
			if (this->data == NULL) {
				return this->data;
			} else {
				this->size *= 2;
				memset(this->data + this->position, 0, this->size - this->position);
			}
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

	bool set_alive(bool alive) {
		return this->alive = alive;
	}

	bool get_alive(void) {
		return this->alive;
	}

	void print(int width = 16) {
		int i = 0;
		if (width < 1) {
			width = 16;
		}

		puts("--- begin (hex) ---");
		while (i < this->position) {
			if (i % width == 0) {
				printf("%p ", this->data + i);
			}
			printf(" %02x", *(char*)(this->data + i));
			i++;
			if (i % width == 0) {
				puts("");
			}
		}
		puts("\n--- end (hex) ---");
		return ;
	}

	~Transport() {
		free(this->data);
		printf("fd = %d, Free %p, %f%% (%d / %d)\n", this->fd, this->data, 100. * this->position / this->size, this->position, this->size);
		memset(this, 0, sizeof *this);
	}
};

