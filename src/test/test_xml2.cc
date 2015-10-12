#include "libxml/parser.h"
#include <iostream>

int main(int argc, char **argv) {
	int ret = 0;

	xmlDocPtr d = NULL;
	xmlNodePtr p = NULL;
	d = xmlReadFile("f1.xml", "UTF-8", XML_PARSER_START_TAG);
	if (d == NULL) {
		return 1;
	}

	p = xmlDocGetRootElement(d);
	if (p == NULL) {
		xmlFreeDoc(d);
		return 2;
	}

//	if (xmlStrcmp(p->name, 

	return ret;
}



