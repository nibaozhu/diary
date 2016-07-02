
#include "connection.h"

class Enter {
private:
	int value;

	int sockfd;

	/* USE for `epoll_wait' */
	int epfd; /* a file descriptor referring to the new epoll instance. */
	struct epoll_event events[MAXEVENTS];
	int maxevents; /* The epoll_wait() system call waits for events on the epoll instance referred to by the file descriptor epfd. The memory area pointed
 to by events will contain the events that will be available for the caller. Up to maxevents are returned by epoll_wait(). The maxevents argument must be greater than zero. */
	int timeout; /* The call waits for a maximum time of timeout milliseconds. Specifying a timeout of -1 makes epoll_wait() wait indefinitely, while
 specifying a timeout equal to zero makes epoll_wait() to return immediately even if no events are available (return code equal to
 zero). */

	std::list<Connection*> *received_message; /* it has been received message from socket */
	std::list<Connection*> *sending_message; /* there is message that will be sending */
	std::map<int, Connection*> *map_connection; /* fd versus connection */

	int receive_from_socket(); /* receive from socket */
	int send_to_socket(); /* send to socket */
	int message_processing(); /* message processing */

	int socket_bind_listen_epoll();
	int set_non_blocking(int sockfd);

public:
	Enter(int value);
	virtual ~Enter();

	int get_value();
	void set_value(int value);

	void working();
};
