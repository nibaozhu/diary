
#include "connection.h"

class Enter {
private:
	int value;

	std::list<Connection*> *received_message; /* it has been received message from socket */
	std::list<Connection*> *sending_message; /* there is message that will be sending */
	std::map<int, Connection*> *map_connection; /* fd versus connection */

	int receive_from_socket(); /* receive from socket */
	int send_to_socket(); /* send to socket */
	int message_processing(); /* message processing */

public:
	Enter(int value);
	virtual ~Enter();

	int get_value();
	void set_value(int value);

	void working();
};
