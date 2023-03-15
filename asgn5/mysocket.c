#include "mysocket.h"
#include <pthread.h>
#include <stdlib.h>

MyFD *my_socket(int __domain,int __type,int __protocol){
    MyFD *__fd = (MyFD *)malloc(sizeof(struct MyFD));
    __fd->sock_fd = socket(__domain,__type,__protocol);
    __fd->flag = 0;
    // TODO: initqueues
    __fd->sendTable;
    __fd->recvTable;

    pthread_mutex_init(&(__fd->recvTableLock),NULL);
    pthread_mutex_init(&(__fd->sendTableLock),NULL);

    if(__fd->sock_fd<0){
        __fd->flag = 1;
    }
    return __fd;
}

void *read_loop(void *args){
    MyFD *__fd = (MyFD *)args;
    while(1){
        if(__fd->flag == 1){
            pthread_exit(0);
        }
        pthread_mutex_lock(&(__fd->recvTableLock));
        // if()
    }
    return 0;
}

void *write_loop(void *args){
    MyFD *__fd =  (MyFD *)args;
    while(1){
        if(__fd->flag == 1){
            pthread_exit(0);
        }

    }
    return 0;
}

int my_bind(MyFD *__fd,const struct sockaddr *__addr,socklen_t __len){
    return bind(__fd->sock_fd,__addr,__len);
}
int my_listen(MyFD *__fd, int __n){
    return listen(__fd->sock_fd,__n);
}
int my_accept(MyFD *__fd,struct sockaddr *__addr,socklen_t *__addr_len){
    if (pthread_create(&(__fd->readThread), NULL, read_loop, __fd) != 0) { // create the read thread
        perror("Error creating readThread thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&(__fd->writeThread), NULL, write_loop, __fd) != 0) { // create the write thread
        perror("Error creating writeThread thread!");
        exit(EXIT_FAILURE);
    }
    return accept(__fd->sock_fd,__addr,__addr_len);
}
int my_connect(MyFD *__fd,struct sockaddr * __addr, socklen_t  __addr_len){
    return connect(__fd->sock_fd,__addr,__addr_len);
}
ssize_t my_send(MyFD *__fd,const void *__buf, size_t __n, int __flags){
    // push in table
}
ssize_t my_recv(MyFD *__fd, void *__buf, size_t __n, int __flags){
    // pop from table
}

int my_close(MyFD *__fd){
    __fd->flag = 1;
    // sleep(5);
    return close(__fd->sock_fd);
}