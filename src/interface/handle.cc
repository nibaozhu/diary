#include "handle.h"

int handle(enum Type t, void *message, int size) {
	int ret = 0;
	switch (t) {
		case json:
			handle_json();
		case protobuf:
			handle_protobuf();
		case xml:
			handle_xml();
		case kv:
			handle_kv();
		default:
			;;;;;;;;;;;
	}

	return ret;
}

int handle_json() {
	int ret = 0;

	return ret;
}

int handle_protobuf() {
	int ret = 0;

	return ret;
}

int handle_xml() {
	int ret = 0;

	return ret;
}

int handle_kv() {
	int ret = 0;

	return ret;
}
