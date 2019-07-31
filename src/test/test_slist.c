#include <stdio.h>
#include <stdlib.h>


typedef struct Node {
	int data;
	struct Node *next;
} NODE, *SLIST;

void printNode(SLIST sl1) {
	NODE *p = sl1;
	int i = 0;
	while (p) {
		printf("node[%d]: %d\n", i++, p->data);
		p = p->next;
	}
}

void reverse1(SLIST *sl1) {
	NODE *p = (*sl1);
	NODE *p2 = NULL;
	NODE *p3 = NULL;

	while (p)
	{
		if (p->next == NULL) {
			(*sl1) = p;
		}

		p2 = p->next;
		p->next = p3;
		p3 = p;
		p = p2;
	}
}

int main(int argc, char **argv) {

	SLIST sl1 = malloc(sizeof (NODE));
	NODE **pp = &sl1->next;
	sl1->data = -1;
	sl1->next = NULL;

	int i;
	for (i = 0; i < 10*1024*1024; i++)
	{
		NODE *node = malloc(sizeof (NODE));
		if (node == NULL) {
			break;
		}
		node->data = i;
		node->next = NULL;

		*pp = node;
		pp = &node->next;
	}

	printNode(sl1);
	reverse1(&sl1);
	printNode(sl1);

	return 0;
}
