
#include "common.h"

class Connection {
private:
	int value;
	std::string str;

public:
	Connection(int value);
	virtual ~Connection();

	int get_value();
	void set_value(int value);
	std::string get_str();
	void set_str(std::string str);
};
