#include "queue.h"
#include <string.h>
#include <stdio.h>

void queue_init(Q* q, int capacity)
{
    q->capacity = capacity;
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
}

void queue_push(Q* q,const char *s)
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
        q->tail = temp;
    }
    q->size++;
    // queue_print(q);
}

void queue_pop(Q* q)
{
    if (q->head == NULL)
        return;
    LL *temp = q->head;
    q->head = q->head->next;
    q->size--;
    if(q->size == 0){
        q->tail = NULL;
    }
    free(temp->data);
    free(temp);
    // queue_print(q);
}

char *queue_front(Q* q)
{
    if(q->head == NULL)
        return NULL;
    return (q->head->data);
}

void queue_print(Q* q)
{
    printf("here7\n");
    LL *tmp = q->head;
    printf("here8\n");
    int i = 0;
    printf("here9\n");
    while(tmp!=NULL){
        i++;
    printf("here0\n");
        if(tmp->data == NULL){
            printf("%d) data : IS NULL :::\n",i);
        }
        else
            printf("%d)data: %s\n",i,tmp->data);
    printf("here1\n");
        tmp = tmp->next;
    printf("here2\n");
    }
    printf("here3\n");

}