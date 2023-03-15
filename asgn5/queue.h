#include <stdlib.h>

typedef struct LL
{
    char *data;
    LL *next;
} LL;

typedef struct Q
{
    LL *head;
    LL *tail;
    int size;
    int capacity;
} Q;

void queue_init(Q* q, int capacity);

void queue_push(Q* q, char *s);

void queue_pop(Q* q);

char* queue_front(Q* q);

void queue_delete(Q* q);
