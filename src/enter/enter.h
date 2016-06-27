class Enter {
private:
	int value;

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
