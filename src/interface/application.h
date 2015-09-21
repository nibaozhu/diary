#ifndef APPLICATION_H
#define APPLICATION_H

enum Type {
	json,
	protobuf,
	xml,
	kv,
};

class Application {
private:
	enum Type t;
	void *message;
	int size;
public:
	Application(enum Type t = kv) {
		this->t = t;
	}

	~Application() {
		;;;;
	}
};


#endif
