#include "mysocket.h"
#include <pthread.h>
#include <stdlib.h>

struct MyFD *my_socket(int __domain,int __type,int __protocol){
    struct MyFD *__fd = (struct MyFD *)malloc(sizeof(struct MyFD));
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

void read_loop(struct MyFD *__fd){
    while(1){
        if(__fd->flag == 1){
            pthread_exit(0);
        }
        pthread_mutex_lock(lock[0]);
        if()
    }
}

void write_loop(){
    while(1){
        if(__fd->flag == 1){
            pthread_exit(0);
        }
    }
}

int my_bind(struct MyFD *__fd,const struct sockaddr *__addr,socklen_t __len){
    return bind(__fd->sock_fd,__addr,__len);
}
int my_listen(struct MyFD *__fd, int __n){
    return listen(__fd->sock_fd,__n);
}
int my_accept(struct MyFD *__fd,struct sockaddr *__addr,socklen_t *__addr_len){
    pthread_create()
    return accept(__fd->sock_fd,__addr,__addr_len);
}
int my_connect(struct MyFD *__fd,struct sockaddr * __addr, socklen_t  __addr_len){
    return connect(__fd->sock_fd,__addr,__addr_len);
}
ssize_t my_send(struct MyFD *__fd,const void *__buf, size_t __n, int __flags){
    // push in table
}
ssize_t my_recv(struct MyFD *__fd, void *__buf, size_t __n, int __flags){
    // pop from table
}

int my_close(struct MyFD *__fd){
    __fd->flag = 1;
    // sleep(5);
    return close(__fd->sock_fd);
}