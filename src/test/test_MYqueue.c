#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct entry {
	struct entry *previous;
	struct entry *next;
	int data;
};

struct queue {
	struct entry *head;
	struct entry *tail;

	size_t length;
};

int push_back(struct queue *q, int data) {

	if (q == NULL) {
		return -1;
	}

	struct entry *e = (struct entry*) malloc(sizeof (struct entry));
	memset(e, 0, sizeof (struct entry));
	e->data = data;

	if (q->tail == NULL) {
		q->tail = e;
		q->head = e;
		q->length = 1;
		printf("PUSH_BACK %d\n", e->data);
	} else {
		e->previous = q->tail; // XXX
		q->tail->next = e;
		q->tail = e;
		q->length += 1;
		printf("push_back %d\n", e->data);
	}

	return 0;
}

int pop_front(struct queue *q) {

	if (q == NULL) {
		return -1;
	}

	struct entry *ptr = NULL;

	if (q->head == NULL) {
		return 0;
	} else {
		q->length -= 1;
		ptr = q->head;
		printf("pop_front %d\n", ptr->data);
		q->head = ptr->next; // XXX

		if (q->head == NULL) {
			q->tail = NULL;
			q->length = 0;
			printf("Has Empty\n");
		} else {
			q->head->previous = NULL;
		}

		free(ptr);
		ptr = NULL;
	}

	return 0;
}


int main() {
	struct queue *q = (struct queue*) malloc(sizeof (struct queue));
	memset(q, 0, sizeof (struct queue));

	push_back(q, 4);
	push_back(q, 5);
	push_back(q, 6);

	pop_front(q);
	pop_front(q);
	pop_front(q);

	return 0;
}
