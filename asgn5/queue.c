#include "queue.h"
#include <string.h>

void queue_init(Q* q, int capacity)
{
    q->capacity = capacity;
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
}

void queue_push(Q* q, char *s)
{
    if (q->size == q->capacity)
        return;

    LL *temp = (LL *)malloc(sizeof(LL));

    int len = strlen(s);
    temp->data = (char *)malloc(sizeof(char) * (len + 1));
    strcpy(temp->data, s);
    temp->data[len] = '\0';

    temp->next = NULL;

    if (q->head == NULL)
        q->tail = q->head = temp;
    else
    {
        q->tail->next = temp;
        q->tail = q->tail->next;
    }

    q->size++;
}

void queue_pop(Q* q)
{
    if (q->head == NULL)
        return;

    LL *temp = q->head;
    q->head = q->head->next;

    q->size--;

    free(temp->data);
    free(temp->next);
}

char *queue_front(Q* q)
{
    if(q->head == NULL)
        return NULL;

    return (q->head->data);
}

void queue_delete(Q* q)
{
    
}