#include "mysocket.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WAITTIME 0.1
#define FRAME_SIZE 10
#define MSG_SIZE 5001

MyFD* initMyFD(int fd){
    MyFD *__fd = (MyFD *)malloc(sizeof(MyFD));
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
    if(__type!=SOCK_MyTCP){
        return initMyFD(-1);
    }
    return initMyFD(socket(__domain, SOCK_STREAM, __protocol));
}

void clear_buffer(char buf[],int n){
    for(int i = 0;i<n;i++){
        buf[i] = '\0';
    }
}

void *read_loop(void *args)
{
    // printf("starting read loopppp\n");
    MyFD *__fd = (MyFD *)args;
    char s[MSG_SIZE];
    while (1)
    {
        // printf("[read loop] here\n");
        if (__fd->flag == 1)
        {
            pthread_exit(0);
        }
        clear_buffer(s,MSG_SIZE);
        pthread_mutex_lock(&(__fd->recvTableLock));
        // printf("[read loop] above recvtable len = %d\n",__fd->recvTable.size);
        while (__fd->recvTable.size == __fd->recvTable.capacity)
        {
            pthread_mutex_unlock(&(__fd->recvTableLock));
            sleep(WAITTIME);
            pthread_mutex_lock(&(__fd->recvTableLock));
        }
        // printf("[read loop]below recvtable len = %d\n",__fd->recvTable.size);
        pthread_mutex_unlock(&(__fd->recvTableLock));
        char chunk[FRAME_SIZE];
        clear_buffer(chunk,FRAME_SIZE);
        int recBits = 0;
        while(1){
            recBits = recv(__fd->sock_fd,chunk,FRAME_SIZE,0);
            // printf("Received %d bytes.\n",recBits);
            if(recBits>0){
                break;
            }
        }
        int recved = 0;
        while(1){
            // printf("Received this chunk : %s[%ld]\n", chunk,strlen(chunk));
            int i;
            for(i=0;;i++){
                s[i+recved] = chunk[i];
                // printf("Pehla loop\n");
                // printf("%c %d\n",chunk[i], chunk[i]);
                if(chunk[i]=='\0'){
                    break;
                }
            }
            recved += i;
            if(i+1<FRAME_SIZE){
                if(chunk[i+1]=='\0'){
                    // printf("Dusra\n");
                    break;
                }
            }

            clear_buffer(chunk,FRAME_SIZE);
            recBits = recv(__fd->sock_fd, chunk, FRAME_SIZE, 0);
            // printf("Received %d bytes.\n",recBits);
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
    // printf("starting write loopppp\n");
    MyFD *__fd = (MyFD *)args;
    char s[MSG_SIZE];
    while (1)
    {
        // printf("[write loop] here\n");
        if (__fd->flag == 1)
        {
            pthread_exit(0);
        }
        clear_buffer(s,MSG_SIZE);
        pthread_mutex_lock(&(__fd->sendTableLock));
        // printf("%d[write loop] above sendtable len = %d\n",__fd->sock_fd,__fd->sendTable.size);
        while(__fd->sendTable.size == 0)
        {
            pthread_mutex_unlock(&(__fd->sendTableLock));
            sleep(WAITTIME);
            pthread_mutex_lock(&(__fd->sendTableLock));
        }
        // printf("%d[write loop] below sendtable len = %d\n",__fd->sock_fd,__fd->sendTable.size);
        const char *src = queue_front(&(__fd->sendTable));
        // printf("src: %s\n",src);
        strcpy(s, src);
        pthread_mutex_unlock(&(__fd->sendTableLock));
        // printf("string to be sent: %s\n",s);
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
            // printf("Sending this chunk: %s[%ld]\n",chunk, strlen(chunk));
            int ret = send(__fd->sock_fd, chunk, chunk_len+1, 0);
            if(ret < 0){
                continue;
            }
            sent += (ret-1);
        }
        pthread_mutex_lock(&(__fd->sendTableLock));
        // printf("not crashed\n");
        queue_pop(&(__fd->sendTable));
        // printf("crashed\n");
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
    MyFD * fd = initMyFD(accept(__fd->sock_fd, __addr, __addr_len));
    if (pthread_create(&(fd->readThread), NULL, read_loop, fd) != 0)
    { // create the read thread
        perror("Error creating readThread thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&(fd->writeThread), NULL, write_loop, fd) != 0)
    { // create the write thread
        perror("Error creating writeThread thread!");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int my_connect(MyFD *__fd, struct sockaddr *__addr, socklen_t __addr_len)
{
    int ret = connect(__fd->sock_fd, __addr, __addr_len);
    if(ret<0){
        return ret;
    }
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
    return ret;
}

ssize_t my_send(MyFD *__fd, const char *__buf, size_t __n, int __flags)
{
    // push in table
    pthread_mutex_lock(&(__fd->sendTableLock));
    // printf("[my send1]sendtable len = %d\n",__fd->sendTable.size);
    while (__fd->sendTable.size == __fd->sendTable.capacity)
    {
        pthread_mutex_unlock(&(__fd->sendTableLock));
        sleep(WAITTIME);
        pthread_mutex_lock(&(__fd->sendTableLock));
    }

    // Restricting Message size in Send_Table to be <= 5000.
    int i;
    char buffer[MSG_SIZE];
    for(i=0;i<MSG_SIZE-1 && __buf[i] != '\0';i++){
        buffer[i] = __buf[i];
    }
    buffer[i] = '\0';
    queue_push(&(__fd->sendTable), buffer);
    // printf("%d[my send2]sendtable len = %d\n",__fd->sock_fd,__fd->sendTable.size);
    pthread_mutex_unlock(&(__fd->sendTableLock));
    return __n;
}

ssize_t my_recv(MyFD *__fd, char *__buf, size_t __n, int __flags)
{
    // pop from table
    clear_buffer(__buf,__n);
    pthread_mutex_lock(&(__fd->recvTableLock));
    // printf("[my recv]recvtable len = %d\n",__fd->recvTable.size);
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