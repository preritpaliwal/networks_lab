#include "mysocket.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WAITTIME 0.1
#define FRAME_SIZE 1000
#define MSG_SIZE 5001

MyFD* initMyFD(int fd){
    MyFD *__fd = (MyFD *)malloc(sizeof(struct MyFD));
    __fd->sock_fd = fd;
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

MyFD *my_socket(int __domain, int __type, int __protocol)
{
    return initMyFD(socket(__domain, __type, __protocol));
}

void clear_buffer(char buf[],int n){
    for(int i = 0;i<n;i++){
        buf[i] = '\0';
    }
}

void *read_loop(void *args)
{
    MyFD *__fd = (MyFD *)args;
    char s[MSG_SIZE];
    while (1)
    {
        if (__fd->flag == 1)
        {
            pthread_exit(0);
        }
        clear_buffer(s,MSG_SIZE);
        pthread_mutex_lock(&(__fd->recvTableLock));
        while (__fd->recvTable.size == __fd->recvTable.capacity)
        {
            pthread_mutex_unlock(&(__fd->recvTableLock));
            sleep(WAITTIME);
            pthread_mutex_lock(&(__fd->recvTableLock));
        }
        pthread_mutex_unlock(&(__fd->recvTableLock));
        char chunk[FRAME_SIZE];
        clear_buffer(chunk,FRAME_SIZE);
        int recBits = 0;
        while(1){
            recBits = recv(__fd->sock_fd,chunk,FRAME_SIZE,0);
            if(recBits>0){
                break;
            }
        }
        int recved = 0;
        while(1){
            int i;
            for(i = 0;;i++){
                s[i+recved] = chunk[i];
                if(chunk[i]=='\0'){
                    break;
                }
            }
            recved += i;
            if(i+1<FRAME_SIZE){
                if(chunk[i+1]=='\0'){
                    break;
                }
            }
            clear_buffer(chunk,FRAME_SIZE);
            recBits = recv(__fd->sock_fd, chunk, FRAME_SIZE, 0);
            if(recBits == 0){
                break;
            }
        }
        pthread_mutex_lock(&(__fd->recvTableLock));
        queue_push(&(__fd->recvTable),s);
        pthread_mutex_unlock(&(__fd->recvTableLock));
    }
    return 0;
}

void *write_loop(void *args)
{
    MyFD *__fd = (MyFD *)args;
    char s[MSG_SIZE];
    while (1)
    {
        if (__fd->flag == 1)
        {
            pthread_exit(0);
        }
        clear_buffer(s,MSG_SIZE);
        pthread_mutex_lock(&(__fd->sendTableLock));
        while(__fd->sendTable.size == 0)
        {
            pthread_mutex_unlock(&(__fd->sendTableLock));
            sleep(WAITTIME);
            pthread_mutex_lock(&(__fd->sendTableLock));
        }
        strcpy(s, queue_front(&(__fd->sendTable)));
        pthread_mutex_unlock(&(__fd->sendTableLock));
        int len = strlen(s);
        int sent = 0;
        while (sent < len)
        {
            char chunk[FRAME_SIZE];
            clear_buffer(chunk,FRAME_SIZE);
            int i;
            for(i=0;i<FRAME_SIZE-1 && s[sent + i] != '\0';i++){
                chunk[i] = s[sent+i];
            }

            chunk[i] = '\0';
            int chunk_len = strlen(chunk);
            sent += send(__fd->sock_fd, chunk, chunk_len, 0);
        }
        pthread_mutex_lock(&(__fd->sendTableLock));
        queue_pop(&(__fd->sendTable));
        pthread_mutex_unlock(&(__fd->sendTableLock));
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

MyFD* my_accept(MyFD *__fd, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len)
{
    printf("near init\n");
    MyFD * fd = initMyFD(accept(__fd->sock_fd, __addr, __addr_len));
    printf("starting\n");
    if (pthread_create(&(fd->readThread), NULL, read_loop, __fd) != 0)
    { // create the read thread
        perror("Error creating readThread thread!");
        exit(EXIT_FAILURE);
    }
    printf("mid\n");
    if (pthread_create(&(fd->writeThread), NULL, write_loop, __fd) != 0)
    { // create the write thread
        perror("Error creating writeThread thread!");
        exit(EXIT_FAILURE);
    }
    printf("done\n");
    return fd;
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
    clear_buffer(__buf,__n);
    pthread_mutex_lock(&(__fd->recvTableLock));
    while (__fd->recvTable.size == 0)
    {
        pthread_mutex_unlock(&(__fd->recvTableLock));
        sleep(WAITTIME);
        pthread_mutex_lock(&(__fd->recvTableLock));
    }

    int i;
    char* src = queue_front(&(__fd->recvTable));
    for(i=0;i<__n-1 && src[i] != '\0';i++){
        __buf[i] = src[i];
    }
    __buf[i] = '\0';

    queue_pop(&(__fd->recvTable));
    pthread_mutex_unlock(&(__fd->recvTableLock));
    int ret = strlen(__buf);
    return ret;
}

int my_close(MyFD *__fd)
{
    sleep(5);
    __fd->flag = 1;
    int ret = close(__fd->sock_fd);
    free(__fd);
    return ret;
}