// NOTE: temporary code
#define DEQUE_MAIN

/* VIM: tabstop=2 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: malloc safe version
#define malloc_s(size)                      \
({                                          \
 void *ptr = malloc(size);                  \
 ptr == NULL ? NULL : memset(ptr, 0, size); \
})                                          \

// NOTE: free safe version
#define free_s(ptr)                         \
do {                                        \
  free(ptr);                                \
  ptr = NULL;                               \
} while (0)                                 \

typedef struct __entry {
  struct __entry *prev;
  struct __entry *next;
  void *data;
} entry;

// NOTE: double ended queue
typedef struct __deque {
  entry *head;
  entry *tail;
  size_t length;
} deque;

int push_back(deque *q, void *data) {
  if (q == NULL) return -1;

  entry *e = (entry*) malloc_s(sizeof (entry));
  if (e == NULL) return -1;
  e->data = data;

  if (q->tail == NULL) {
    q->tail = q->head = e;
  } else {
    e->prev = q->tail;
    q->tail = q->tail->next = e;
  }

  q->length++;
  return 0;
}

int pop_front(deque *q, void **data) {
  if (q == NULL) return -1;
  if (q->head == NULL) return 0;
 
  entry *e = q->head;
  if (e->next == NULL) {
    q->tail = q->head = NULL;
  } else {
    q->head = e->next;
    q->head->prev = NULL;
  }

  *data = e->data;
  free_s(e);
  q->length--;
  return 0;
}

#ifdef DEQUE_MAIN
int main() {
  deque *q = (deque*) malloc_s(sizeof (deque));

  unsigned int seed = getpid();
  srand(seed);

  int i = 0;
  int rd = 0;
  void *data = NULL;
  for (i = 0; i < (1<<20); i++)
  {
    rd = rand();
    data = malloc_s(sizeof (int));
    if (data == NULL) continue;
    memcpy(data, &rd, sizeof (int));

    push_back(q, data);
  }

  for (i = 0; i < (1<<23); i++)
  {
    pop_front(q, &data);
    free_s(data);
  }

  free_s(q);
  return 0;
}
#endif
