#include <stdio.h>
#include <stdlib.h>

struct entry {
	int data;
	struct entry *next;
};

struct stack {
	struct entry *top;
	size_t length;
};

int init_stack(struct stack *s) {
	if (!s) {
		return -1;
	}
	s->top = NULL;
	s->length = 0;
	return 0;
}

int push_back(struct stack *s, int data) {
	if (!s) {
		return -1;
	}

	struct entry *e = (struct entry*) malloc (sizeof (struct entry));
	e->data = data;
	e->next = s->top;
	s->top = e;
	s->length++;

	return 0;
}

int pop_back(struct stack *s, int *data) {
	if (!s) {
		return -1;
	}

	if (s->top == NULL || s->length == 0) {
		*data = 0;
		return 0;
	}

	struct entry *e = NULL;
	e = s->top;
	*data = e->data;
	s->top = e->next;
	s->length--;
	free (e);
	e = NULL;

	return 0;
}

int main(int argc, char **argv) {

	struct stack *s = (struct stack*) malloc (sizeof (struct stack));

	init_stack(s);

	push_back(s, 3);
	push_back(s, 4);
	push_back(s, 5);

	int data = 0;
	pop_back(s, &data);
	printf("%d\n", data);

	pop_back(s, &data);
	printf("%d\n", data);

	pop_back(s, &data);
	printf("%d\n", data);

	pop_back(s, &data);
	printf("%d\n", data);

	return 0;
}
