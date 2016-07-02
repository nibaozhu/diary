#include "enter.h"
#include "common.h"

Enter::Enter(int value): value(value) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	this->maxevents = MAXEVENTS;
	this->timeout = 1000; /* in milliseconds */

	this->received_message = new std::list<Connection*>();
	this->sending_message = new std::list<Connection*>();
	this->map_connection = new std::map<int, Connection*>();

#if DEBUG
	std::clog
					<< "this = " << this 
					<< ", received_message = " << this->received_message
					<< ", sending_message = " << this->sending_message
					<< ", map_connection = " << this->map_connection
					<< std::endl;

	std::clog << "this->value = " << this->value << std::endl;
#endif

	int ret = this->socket_bind_listen_epoll();
	if (ret == -1) {
		throw ERROR_socket_bind_listen_epoll;
	}

	return ;
}

Enter::~Enter() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog
					<< "this = " << this 
					<< ", received_message = " << this->received_message
					<< ", sending_message = " << this->sending_message
					<< ", map_connection = " << this->map_connection
					<< std::endl;

	int fd = 0;
	Connection *connection = NULL;
	int ret = 0;


	std::map<int, Connection*>::iterator it = this->map_connection->begin();
	for ( ; it != this->map_connection->end(); it++ ) {
		fd = it->first;

		ret = close(fd);
		if (ret == -1) {
			std::clog << strerror(errno) << std::endl;
			continue;	/* XXX: ignore error */
		}

		connection = it->second;
		if (connection) {
			delete connection;
		}
	}

	delete this->received_message;
	delete this->sending_message;
	delete this->map_connection;

	std::clog << "this->value = " << this->value << std::endl;
#endif

	return ;
}

int Enter::get_value() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return this->value;
}

void Enter::set_value(int value) {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;
	this->value = value;

#if DEBUG
	std::clog << "this->value = " << this->value << std::endl;
#endif

	return ;
}

void Enter::working() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	do {

		if (this->value > 0) {
			std::clog << "this->value = " << this->value << std::endl;
			this->value --;
		} else {
			std::clog << "this->value = " << this->value << " break;" << std::endl;
			break;
		}

		this->receive_from_socket();
		this->send_to_socket();
		this->message_processing();

	} while (true);

	return ;
}

int Enter::receive_from_socket() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	int the_number_of_file_descriptors_ready_for_the_requested_IO = epoll_wait(this->epfd, this->events, this->maxevents, this->timeout);
	if (the_number_of_file_descriptors_ready_for_the_requested_IO == -1) {
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

	for (int i = 0; i < the_number_of_file_descriptors_ready_for_the_requested_IO; i++) {
		if (this->events[i].data.fd == this->sockfd) { /* listened socket */

			struct sockaddr_in peer_addr;
			socklen_t addrlen;
			int the_accepted_socket = accept(this->sockfd, (struct sockaddr *)&peer_addr, &addrlen);
			if (the_accepted_socket == -1) {
				std::cerr << strerror(errno) << std::endl;
				return -1;
			}

			int ret = this->set_non_blocking(the_accepted_socket);
			if (ret == -1) {
				std::cerr << strerror(errno) << std::endl;
				return -1;
			}

			int op = EPOLL_CTL_ADD;
			int fd = the_accepted_socket;
			struct epoll_event event;
			event.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
			event.data.fd = the_accepted_socket;
			ret = epoll_ctl(this->epfd, op, fd, &event);
			if (ret == -1) {
				std::cerr << strerror(errno) << std::endl;
				return -1;
			}

			// TODO: add to Connection Object.
		} else {

			// TODO: somethings are happended.
			std::clog << "somethings are happended." << std::endl;
			if (this->events[i].events & EPOLLIN) {
				std::clog << "The associated file = " << this->events[i].data.fd << " is available for read(2) operations." << std::endl;

				// XXX: temporarily code
				int sockfd = this->events[i].data.fd;
				void *buf = malloc(BUFSIZ * sizeof (char*));
				size_t len = BUFSIZ * sizeof (char);
				int flags = MSG_DONTWAIT;
				ssize_t the_number_of_bytes_received = recv(sockfd, buf, len, flags);
				if (the_number_of_bytes_received == -1) {
					free(buf);
					std::cerr << strerror(errno) << std::endl;
					return -1;
				}

				std::clog << "RECEIVED buf [" << (char*)buf << "]" << std::endl;
				free(buf);
			}

		}
	} /* for-loop-end */

	return 0;
}

int Enter::send_to_socket() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	return 0;
}

int Enter::message_processing() {
	std::clog << __FILE__ << ": " << __LINE__ << ": " << __func__ << std::endl;

	return 0;
}

int Enter::socket_bind_listen_epoll() {

	int domain = AF_INET;
	int type = SOCK_STREAM;
	int protocol = 0;

	this->sockfd = socket(domain, type, protocol);
	if (this->sockfd == -1) {
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof (struct sockaddr_in);

	addr.sin_port = htons(12340);
	const char *ip = "0.0.0.0";

	int af = domain;
	const char *src = ip;
	int ret = inet_pton(af, src, (struct sockaddr *) &addr.sin_addr.s_addr);
	if (ret != 1) {
		std::cerr << strerror(errno) << std::endl;
	  return -1;
	}

	ret = bind(this->sockfd, (const struct sockaddr*) &addr, addrlen);
	if (ret == -1) {
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

	int backlog = 1;
	ret = listen(this->sockfd, backlog);
	if (ret == -1) {
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}


	int size = (1<<4);
	this->epfd = epoll_create(size);
	if (this->epfd == -1) {
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

#if DEBUG
	std::clog << "this->epfd = " << this->epfd << std::endl;
#endif

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP; /* Level Triggered */
	event.data.fd = this->sockfd;
	int op = EPOLL_CTL_ADD;
	int fd = this->sockfd;
	ret = epoll_ctl(epfd, op, fd, &event);
	if (ret == -1) {
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

	return 0;
}

int Enter::set_non_blocking(int sockfd) {
  int ret = 0;
  do {
    int flags = fcntl(sockfd, F_GETFL);
    if (flags == -1) {
			std::cerr << strerror(errno) << std::endl;
      ret = -1;
      break;
    }
    flags |= O_NONBLOCK; /* If the O_NONBLOCK flag is enabled, then the system call fails with the error EAGAIN. */
		std::clog << "Set the file = " << sockfd << " status flags to the value = 0" << std::oct << flags << "." << std::endl;
																																							/* TODO: octal */
		std::clog << std::dec;

    ret = fcntl(sockfd, F_SETFL, flags);
    if (ret == -1) {
			std::cerr << strerror(errno) << std::endl;
      break;
    }
  } while (false);
  return ret;
}

