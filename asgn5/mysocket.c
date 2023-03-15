#include "mysocket.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define WAITTIME 0.1
#define CHUNK_SIZE 1000
#define MSG_SIZE 5001

MyFD *my_socket(int __domain, int __type, int __protocol)
{
    MyFD *__fd = (MyFD *)malloc(sizeof(struct MyFD));
    __fd->sock_fd = socket(__domain, __type, __protocol);
    __fd->flag = 0;
    // TODO: initqueues
    queue_init(&(__fd->sendTable), 10);
    queue_init(&(__fd->recvTable), 10);

    pthread_mutex_init(&(__fd->recvTableLock), NULL);
    pthread_mutex_init(&(__fd->sendTableLock), NULL);

    if (__fd->sock_fd < 0)
    {
        __fd->flag = 1;
    }
    return __fd;
}

void *read_loop(void *args)
{
    MyFD *__fd = (MyFD *)args;
    while (1)
    {
        if (__fd->flag == 1)
        {
            pthread_exit(0);
        }
        if (__fd->recvTable.size < __fd->recvTable.capacity)
        {
            char *s = (char *)malloc(sizeof(char) * MSG_SIZE);

            free(s);
        }
    }
    return 0;
}

void *write_loop(void *args)
{
    MyFD *__fd = (MyFD *)args;
    while (1)
    {
        if (__fd->flag == 1)
        {
            pthread_exit(0);
        }
        sleep(WAITTIME);
        if (__fd->sendTable.size > 0)
        {
            char *s = (char *)malloc(sizeof(char) * MSG_SIZE);
            strcpy(s, queue_front(&(__fd->sendTable)));
            int len = strlen(s);
            int sent = 0;

            while (sent < len)
            {

                char *chunk = (char *)malloc(sizeof(char) * CHUNK_SIZE);
                strnpcy(chunk, s+sent, CHUNK_SIZE - 1);
                // chunk[CHUNK_SIZE-1] = '\x03';
                chunk[CHUNK_SIZE-1] = '\0';
                int chunk_len = strlen(chunk);

                // Handle case when remaining less than CHUNK_SIZE.

                int n = send(__fd->sock_fd, chunk, chunk_len, 0);
                sent += n;
            }

            free(s);
        }
    }
    return 0;
}

int my_bind(MyFD *__fd, const struct sockaddr *__addr, socklen_t __len)
{
    return bind(__fd->sock_fd, __addr, __len);
}

int my_listen(MyFD *__fd, int __n)
{
    return listen(__fd->sock_fd, __n);
}

int my_accept(MyFD *__fd, struct sockaddr *__addr, socklen_t *__addr_len)
{
    if (pthread_create(&(__fd->readThread), NULL, read_loop, __fd) != 0)
    { // create the read thread
        perror("Error creating readThread thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&(__fd->writeThread), NULL, write_loop, __fd) != 0)
    { // create the write thread
        perror("Error creating writeThread thread!");
        exit(EXIT_FAILURE);
    }
    return accept(__fd->sock_fd, __addr, __addr_len);
}

int my_connect(MyFD *__fd, struct sockaddr *__addr, socklen_t __addr_len)
{
    return connect(__fd->sock_fd, __addr, __addr_len);
}

ssize_t my_send(MyFD *__fd, const char *__buf, size_t __n, int __flags)
{
    // push in table
    pthread_mutex_lock(&(__fd->sendTableLock));
    while (__fd->sendTable.size == __fd->sendTable.capacity)
    {
        pthread_mutex_unlock(&(__fd->sendTableLock));
        sleep(WAITTIME);
        pthread_mutex_lock(&(__fd->sendTableLock));
    }
    queue_push(&(__fd->sendTable), __buf);
    pthread_mutex_unlock(&(__fd->sendTableLock));
    return __n;
}

ssize_t my_recv(MyFD *__fd, char *__buf, size_t __n, int __flags)
{
    // pop from table

    while (__fd->recvTable.size == 0)
    {
        sleep(WAITTIME);
    }
    strncpy(__buf, queue_front(&(__fd->recvTable)), __n);
    queue_pop(&(__fd->recvTable));
    int ret = strlen(__buf);
    return ret;
}

int my_close(MyFD *__fd)
{
    __fd->flag = 1;
    // sleep(5);
    return close(__fd->sock_fd);
}